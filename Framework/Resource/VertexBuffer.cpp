//
// Created by Glodxy on 2021/10/14.
//

#include "Framework/Resource/include/VertexBuffer.h"
#include "Utils/OGLogging.h"
#include "ResourceAllocator.h"

namespace our_graph {


struct VertexBuffer::Detail {
  AttributeData attributes[MAX_VERTEX_ATTRIBUTE_COUNT];
  std::bitset<32> declared_attributes;
  uint32_t vertex_count = 0;
  uint8_t buffer_count = 0;
  bool enable_buffer_object = false;
  Driver* driver = nullptr;
};

VertexBuffer::Builder::Builder(Driver* driver) noexcept : BuilderBase<Detail>() {
  impl_->driver = driver;
}
VertexBuffer::Builder::~Builder() noexcept  = default;
VertexBuffer::Builder::Builder(const Builder &rhs) noexcept = default;
VertexBuffer::Builder::Builder(Builder &&rhs) noexcept = default;
VertexBuffer::Builder &VertexBuffer::Builder::operator=(Builder &&rhs) noexcept = default;
VertexBuffer::Builder &VertexBuffer::Builder::operator=(const Builder &rhs) noexcept = default;

VertexBuffer::Builder &VertexBuffer::Builder::VertexCount(uint32_t vertex_count) noexcept {
  impl_->vertex_count = vertex_count;
  return *this;
}

VertexBuffer::Builder &VertexBuffer::Builder::EnableBufferObjects(bool enabled) noexcept {
  impl_->enable_buffer_object = enabled;
  return *this;
}

VertexBuffer::Builder &VertexBuffer::Builder::BufferCount(uint8_t buffer_count) noexcept {
  impl_->buffer_count = buffer_count;
  return *this;
}

VertexBuffer::Builder &VertexBuffer::Builder::Attribute(VertexAttribute attribute,
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
  return ResourceAllocator::Get().CreateVertexBuffer(*this);
}

VertexBuffer::VertexBuffer(const Builder &builder)
  : vertex_count_(builder->vertex_count), buffer_count_(builder->buffer_count),
  enable_buffer_objects_(builder->enable_buffer_object),
  driver_(builder->driver) {
  std::copy(std::begin(builder->attributes),
            std::end(builder->attributes),
            attributes_.begin());

  declared_attributes_ = builder->declared_attributes;
  uint8_t attribute_count = (uint8_t)declared_attributes_.count();
  AttributeArray  attribute_array;
  static_assert(attribute_array.size() == MAX_VERTEX_ATTRIBUTE_COUNT,
      "Attribute array size not match size in driver!");

  static_assert(sizeof(AttributeData) == sizeof(Attribute),
      "AttributeData size not match Attribute!");

  size_t buffer_size[MAX_VERTEX_BUFFER_COUNT] = {};

#pragma nounroll
  for (int i = 0; i < attribute_array.size(); ++i) {
    if (declared_attributes_[i]) {
      const uint32_t offset = attributes_[i].offset;
      const uint8_t stride = attributes_[i].stride;
      const uint8_t slot = attributes_[i].buffer;

      attribute_array[i].offset = offset;
      attribute_array[i].stride = stride;
      attribute_array[i].buffer = slot;
      attribute_array[i].type = attributes_[i].type;
      attribute_array[i].flags = attributes_[i].flags;

      const size_t end = offset + vertex_count_ * stride;
      // 设置buffer的大小
      buffer_size[slot] = std::max(buffer_size[slot], end);
    }
  }

  // 该位置为骨骼的索引
  attribute_array[VertexAttribute::BONE_INDICES].flags |= Attribute::FLAG_INTEGER_TARGET;

  handle_ = driver_->CreateVertexBuffer(
      buffer_count_, attribute_count, vertex_count_, attribute_array);

  // 未启用时需要自动创建
  if (!enable_buffer_objects_) {
#pragma nounroll
    for (int i = 0; i < MAX_VERTEX_BUFFER_COUNT; ++i) {
      if (buffer_size[i] > 0) {
        BufferObjectHandle  boh = driver_->CreateBufferObject(
            buffer_size[i], BufferObjectBinding::VERTEX,
            BufferUsage::STATIC);
        driver_->SetVertexBufferObject(handle_, i, boh);
        buffers_[i] = boh;
      }
    }
  }
}

void VertexBuffer::Destroy() {
  if (!enable_buffer_objects_) {
    for (BufferObjectHandle boh : buffers_) {
      driver_->DestroyBufferObject(boh);
    }
  }
  driver_->DestroyVertexBuffer(handle_);
}

size_t VertexBuffer::GetVertexCount() const noexcept {
  return vertex_count_;
}

void VertexBuffer::SetBufferAt(uint8_t buffer_index,
                               BufferDescriptor &&buffer,
                               uint32_t byte_offset) {
  if (enable_buffer_objects_) {
    LOG_ERROR("VertexBuffer", "Should Call SetBufferObjectAt!");
    assert(enable_buffer_objects_);
  }

  if (buffer_index < buffer_count_) {
    assert(buffers_[buffer_index]);
    driver_->UpdateBufferObject(buffers_[buffer_index],
                                std::move(buffer), byte_offset);
  } else {
    LOG_ERROR("VertexBuffer", "BufferIndex Error!"
                              "index:{}, buffer_count:{}",
                              buffer_index, buffer_count_);
    assert(buffer_index < buffer_count_);
  }
}

void VertexBuffer::SetBufferObjectAt(
    uint8_t buffer_index, BufferObjectHandle buffer) {
  if (!enable_buffer_objects_) {
    LOG_ERROR("VertexBuffer", "Should Call SetBufferObjectAt!");
    assert(!enable_buffer_objects_);
  }

  if (buffer_index < buffer_count_) {
    driver_->SetVertexBufferObject(handle_, buffer_index, buffer);
  } else {
    LOG_ERROR("VertexBuffer", "BufferIndex Error!"
                              "index:{}, buffer_count:{}",
              buffer_index, buffer_count_);
    assert(buffer_index < buffer_count_);
  }
}

}  // namespace our_graph