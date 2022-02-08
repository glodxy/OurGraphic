//
// Created by Glodxy on 2021/12/5.
//

#include "UniformBlock.h"
#include "Utils/OGLogging.h"
namespace our_graph {
/*-------------Builder---------------*/
UniformBlock::Builder & UniformBlock::Builder::Name(const std::string &name) {
  name_ = name;
  return *this;
}

UniformBlock::Builder & UniformBlock::Builder::Add(const std::string &name, size_t size, Type type) {
  entries_.emplace_back(name, size, type);
  return *this;
}

UniformBlock UniformBlock::Builder::Build() {
  return UniformBlock(*this);
}
/*----------------------------------*/

UniformBlock::UniformBlock(const UniformBlock::Builder& builder) : name_(builder.name_) {
  auto& info_map = info_map_;
  auto& uniforms_list = uniform_info_list_;
  info_map.reserve(builder.entries_.size());
  uniforms_list.resize(builder.entries_.size());

  uint32_t i = 0;
  uint32_t offset = 0;
  for (const auto& entry : builder.entries_) {
    size_t alignment = ComputeAlignmentForType(entry.type);
    uint8_t stride = ComputeStrideForType(entry.type);

    if (entry.size > 1) {
      alignment = (alignment + 3) & ~3;
      stride = (stride + uint8_t(3)) & ~uint8_t(3);
    }

    size_t padding = (alignment - (offset % alignment)) % alignment;
    offset += padding;

    UniformInfo& info = uniforms_list[i];
    info = {entry.name, offset, stride, entry.type, entry.size,};
    info_map[entry.name] = i;

    offset += stride * entry.size;
    ++i;
  }
  size_ = offset;
}

int64_t UniformBlock::GetUniformOffset(const std::string &name, size_t idx) const noexcept {
  const auto* info = GetUniformInfo(name);
  if (!info) {
    return -1;
  }
  return info->GetBufferOffset(idx);
}

const UniformBlock::UniformInfo *UniformBlock::GetUniformInfo(const std::string &idx) const noexcept {
  const auto& iter = info_map_.find(idx);
  if (iter == info_map_.end()) {
    LOG_ERROR("UniformBlock", "GetInfo Failed! [{}] Cannot Find In Map",
              idx);
    return nullptr;
  }
  return &uniform_info_list_[iter->second];
}

uint8_t UniformBlock::ComputeAlignmentForType(Type type) noexcept {
  switch (type) {
    case Type::BOOL:
    case Type::FLOAT:
    case Type::INT:
    case Type::UINT:
      return 4;
    case Type::BOOL2:
    case Type::FLOAT2:
    case Type::INT2:
    case Type::UINT2:
      return 8;
    case Type::BOOL3:
    case Type::BOOL4:
    case Type::FLOAT3:
    case Type::FLOAT4:
    case Type::INT3:
    case Type::INT4:
    case Type::UINT3:
    case Type::UINT4:
    case Type::MAT3:
    case Type::MAT4:
      return 16;
  }
}

uint8_t UniformBlock::ComputeStrideForType(Type type) noexcept {
  switch (type) {
    case Type::BOOL:
    case Type::FLOAT:
    case Type::INT:
    case Type::UINT:
      return 4;
    case Type::BOOL2:
    case Type::FLOAT2:
    case Type::INT2:
    case Type::UINT2:
      return 8;
    case Type::BOOL3:
    case Type::FLOAT3:
    case Type::INT3:
    case Type::UINT3:
      return 16;
    case Type::BOOL4:
    case Type::FLOAT4:
    case Type::INT4:
    case Type::UINT4:
      return 16;
    case Type::MAT3:
      return 64;
    case Type::MAT4:
      return 64;
  }
}


}  // namespace our_graph