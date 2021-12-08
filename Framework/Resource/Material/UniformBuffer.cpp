//
// Created by Glodxy on 2021/12/7.
//

#include "UniformBuffer.h"

namespace our_graph {
UniformBuffer::UniformBuffer(size_t size) noexcept :
  buffer_(storage_),
  size_(size),
  dirty_(true) {
  if (size > sizeof(storage_)) {
    buffer_ = ::malloc(size);
  }
  memset(buffer_, 0, size);
}


UniformBuffer::UniformBuffer(UniformBuffer &&r) noexcept
    : buffer_(r.buffer_),
      size_(r.size_),
      dirty_(r.dirty_) {
  if (r.IsLocalStorage()) {
    buffer_ = storage_;
    memcpy(buffer_, r.buffer_, size_);
  }
  r.buffer_ = nullptr;
  r.size_ = 0;
}

UniformBuffer &UniformBuffer::operator=(UniformBuffer &&r) noexcept {
  if (this != &r) {
    dirty_ = r.dirty_;
    if (r.IsLocalStorage()) {
      buffer_ = storage_;
      size_ = r.size_;
      memcpy(buffer_, r.buffer_, r.size_);
    } else {
      std::swap(buffer_, r.buffer_);
      std::swap(size_, r.size_);
    }
  }
  return *this;
}

UniformBuffer::~UniformBuffer() noexcept {
  if (buffer_ && !IsLocalStorage()) {
    ::free(buffer_);
  }
}

UniformBuffer &UniformBuffer::SetUniforms(const UniformBuffer &r) noexcept {
  if (this != &r) {
    if (size_ != r.size_) {
      if (buffer_ && !IsLocalStorage()) {
        ::free(buffer_);
      }
      buffer_ = storage_;
      size_ = r.size_;
      if (size_ > sizeof(storage_)) {
        buffer_ = ::malloc(size_);
      }
    }
    memcpy(buffer_, r.buffer_, r.size_);
    Invalidate();
  }
  return *this;
}


template<size_t Size>
void UniformBuffer::SetUniformUntyped(size_t offset, const void* v) noexcept {
  SetUniformUntyped<Size>(InvalidateUniforms(offset, Size), 0ul, v);
}

template
void UniformBuffer::SetUniformUntyped<4ul>(size_t offset, const void* v) noexcept;
template
void UniformBuffer::SetUniformUntyped<8ul>(size_t offset, const void* v) noexcept;
template
void UniformBuffer::SetUniformUntyped<12ul>(size_t offset, const void* v) noexcept;
template
void UniformBuffer::SetUniformUntyped<16ul>(size_t offset, const void* v) noexcept;
template
void UniformBuffer::SetUniformUntyped<64ul>(size_t offset, const void* v) noexcept;

template<size_t Size>
void UniformBuffer::SetUniformArrayUntyped(size_t offset, const void* begin, size_t count) noexcept {
  constexpr size_t stride = (Size + 0xFu) & ~0xFu;
  size_t array_size = stride * count - stride + Size;
  void* p = InvalidateUniforms(offset, array_size);
  for (size_t i = 0; i < count; i++) {
    SetUniformUntyped<Size>(p, 0ul, static_cast<const char *>(begin) + i * Size);
    p = (void*)(uintptr_t(p) + uintptr_t(Size));
  }
}

template
void UniformBuffer::SetUniformArrayUntyped<4ul>(size_t offset, const void* begin, size_t count) noexcept;
template
void UniformBuffer::SetUniformArrayUntyped<8ul>(size_t offset, const void* begin, size_t count) noexcept;
template
void UniformBuffer::SetUniformArrayUntyped<12ul>(size_t offset, const void* begin, size_t count) noexcept;
template
void UniformBuffer::SetUniformArrayUntyped<16ul>(size_t offset, const void* begin, size_t count) noexcept;
template
void UniformBuffer::SetUniformArrayUntyped<64ul>(size_t offset, const void* begin, size_t count) noexcept;


}