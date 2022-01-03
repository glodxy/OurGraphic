//
// Created by Glodxy on 2022/1/3.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TYPEDUNIFORMBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TYPEDUNIFORMBUFFER_H_
#include "Backend/include/BufferDescriptor.h"
#include "Backend/include/Driver.h"
namespace our_graph {

template<typename T, size_t N = 1>
class TypedUniformBuffer {
 public:

  T& At(size_t i) noexcept {
    dirty_ = true;
    return buffer_[i];
  }

  const T& Get(size_t i = 0) const noexcept {
    return buffer_[i];
  }

  // bytes
  size_t GetSize() const noexcept { return sizeof(T) * N; }

  bool IsDirty() const noexcept { return dirty_; }

  // 清除dirty
  void Clean() const noexcept { dirty_ = false; }



  BufferDescriptor ToBufferDescriptor(Driver* driver) const noexcept {
    return ToBufferDescriptor(driver, 0, GetSize());
  }

  // 拷贝数据到driver
  BufferDescriptor ToBufferDescriptor(
     Driver* driver , size_t offset, size_t size) const noexcept {
    BufferDescriptor p;
    p.size_ = size;
    p.buffer_ = driver->Allocate(p.size_);
    memcpy(p.buffer_, reinterpret_cast<const char*>(buffer_) + offset, p.size_); // inlined
    Clean();
    return p;
  }

 private:
  T buffer_[N];
  mutable bool dirty_ = true;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TYPEDUNIFORMBUFFER_H_
