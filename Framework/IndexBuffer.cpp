//
// Created by Glodxy on 2021/10/16.
//

#include "include/IndexBuffer.h"
#include "ResourceAllocator.h"

namespace our_graph {

struct IndexBuffer::Detail {
  uint32_t index_count = 0;
  IndexType index_type = IndexType::UINT;
  Driver* driver = nullptr;
};


IndexBuffer::Builder::Builder(Driver* driver) noexcept : BuilderBase<Detail>() {
  impl_->driver = driver;
}
IndexBuffer::Builder::~Builder() noexcept  = default;
IndexBuffer::Builder::Builder(const Builder &rhs) noexcept = default;
IndexBuffer::Builder::Builder(Builder &&rhs) noexcept = default;
IndexBuffer::Builder &IndexBuffer::Builder::operator=(Builder &&rhs) noexcept = default;
IndexBuffer::Builder &IndexBuffer::Builder::operator=(const Builder &rhs) noexcept = default;

IndexBuffer::Builder &IndexBuffer::Builder::IndexCount(uint8_t index_count) noexcept {
  impl_->index_count = index_count;
  return *this;
}

IndexBuffer::Builder &IndexBuffer::Builder::BufferType(IndexType index_type) noexcept {
  impl_->index_type = index_type;
  return *this;
}

IndexBuffer *IndexBuffer::Builder::Build() {
  return ResourceAllocator::Get().CreateIndexBuffer(*this);
}

IndexBuffer::IndexBuffer(const Builder &builder)
  : index_count_(builder->index_count),
    driver_(builder->driver) {
  handle_ = driver_->CreateIndexBuffer(
      (ElementType)builder->index_type,
      uint32_t(builder->index_count),
      BufferUsage::STATIC);
}

void IndexBuffer::Destroy() {
  driver_->DestroyIndexBuffer(handle_);
}

void IndexBuffer::SetBuffer(
    BufferDescriptor &&data, uint32_t byte_offset) {
  driver_->UpdateIndexBuffer(handle_, std::move(data), byte_offset);
}
}  // namespace our_graph