//
// Created by Glodxy on 2021/10/14.
//

#include "include/VertexBuffer.h"
#include "Utils/OGLogging.h"

namespace our_graph {
struct AttributeData : public Attribute {
  AttributeData() : Attribute{.type = ElementType::FLOAT4} {

  }
};

struct VertexBuffer::Detail {
  AttributeData attributes[MAX_VERTEX_ATTRIBUTE_COUNT];
  std::bitset<32> declared_attributes;
  uint32_t vertex_count = 0;
  uint8_t buffer_count = 0;
  bool enable_buffer_object = false;
};

VertexBuffer::Builder::Builder() noexcept = default;
VertexBuffer::Builder::~Builder() noexcept  = default;
VertexBuffer::Builder::Builder(const Builder &rhs) noexcept = default;
VertexBuffer::Builder::Builder(Builder &&rhs) noexcept = default;
Builder &VertexBuffer::Builder::operator=(Builder &&rhs) noexcept = default;
Builder &VertexBuffer::Builder::operator=(const Builder &rhs) noexcept = default;

Builder &VertexBuffer::Builder::VertexCount(uint32_t vertex_count) noexcept {
  impl_->vertex_count = vertex_count;
  return *this;
}

Builder &VertexBuffer::Builder::EnableBufferObjects(bool enabled) noexcept {
  impl_->enable_buffer_object = enabled;
  return *this;
}

Builder &VertexBuffer::Builder::BufferCount(uint8_t buffer_count) noexcept {
  impl_->buffer_count = buffer_count;
  return *this;
}

Builder &VertexBuffer::Builder::Attribute(VertexAttribute attribute,
                                          uint8_t buffer_index,
                                          AttributeType attribute_type,
                                          uint32_t byte_offset,
                                          uint8_t byte_stride) noexcept {
  size_t attribute_size = DriverApi::GetElementTypeSize(attribute_type);
  if (byte_stride == 0) {
    // 间隔为0时，即逐元素读取
    byte_stride = (uint8_t) attribute_size;
  }

  if (size_t(attribute) < MAX_VERTEX_ATTRIBUTE_COUNT &&
      size_t(buffer_index) < MAX_VERTEX_ATTRIBUTE_COUNT) {
    // 满足条件时加入

    AttributeData& data = impl_->attributes[attribute];
    data.buffer = buffer_index;
    data.offset = byte_offset;
    data.stride = byte_stride;
    data.type = attribute_type;
    impl_->declared_attributes.set(attribute);
  } else {
    LOG_WARN("VertexBuffer Builder","out of limits! [{}] attribute try to set.", attribute);
  }
  return *this;
}

VertexBuffer *VertexBuffer::Builder::Build() {
  if (impl_->vertex_count <= 0) {
    LOG_ERROR("VertexBuffer Builder", "Vertex Count Error:{}", impl_->vertex_count);
    return nullptr;
  }

  if (impl_->buffer_count <= 0 || impl_->buffer_count > MAX_VERTEX_ATTRIBUTE_COUNT) {
    LOG_ERROR("VertexBuffer Builder", "BufferCount Count Error:{}", impl_->buffer_count);
    return nullptr;
  }

  const auto& declared_attributes = impl_->declared_attributes;
  const auto& attributes = impl_->attributes;
  std::bitset<32> attribute_buffers; // 用于记录哪些buffer被使用
  for (int i = 0; i < MAX_VERTEX_ATTRIBUTE_COUNT; ++i) {
    if (declared_attributes[i]) {
      attribute_buffers.set(attributes[i].buffer);
    }
  }

  if (attribute_buffers.count() != impl_->buffer_count) {
    LOG_ERROR("VertexBuffer Builder", "There are buffers not bind to attributes!");
    return nullptr;
  }

  //todo: create
}

}  // namespace our_graph