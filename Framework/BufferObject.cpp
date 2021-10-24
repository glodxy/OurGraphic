//
// Created by Glodxy on 2021/10/17.
//

#include "include/BufferObject.h"
#include "ResourceAllocator.h"
namespace our_graph {
struct BufferObject::Detail {
  BufferObjectBinding binding_type = BufferObjectBinding::UNIFORM;
  uint32_t  byte_count = 0;
  Driver* driver = nullptr;
};

using BuilderType = BufferObject;
BuilderType::Builder::Builder(Driver* driver) noexcept {
  impl_->driver = driver;
}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

BufferObject::Builder &BufferObject::Builder::Size(uint32_t byte_count) noexcept {
  impl_->byte_count = byte_count;
  return *this;
}

BufferObject::Builder &BufferObject::Builder::BindingType(BufferObjectBinding bind_type) noexcept {
  impl_->binding_type = bind_type;
  return *this;
}

BufferObject *BufferObject::Builder::Build() {
  return ResourceAllocator::Get().CreateBufferObject(*this);
}

BufferObject::BufferObject(const Builder &builder)
  : byte_count_(builder->byte_count),
    binding_type_(builder->binding_type),
    driver_(builder->driver) {
  handle_ = driver_->CreateBufferObject(builder->byte_count,
                                        builder->binding_type,
                                        BufferUsage::STATIC);
}

void BufferObject::Destroy() {
  driver_->DestroyBufferObject((handle_));
}

void BufferObject::SetBuffer(
    BufferDescriptor &&buffer, uint32_t byte_offset) {
  driver_->UpdateBufferObject(handle_, std::move(buffer), byte_offset);
}

}  // namespace our_graph