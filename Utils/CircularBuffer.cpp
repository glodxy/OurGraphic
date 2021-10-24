//
// Created by Glodxy on 2021/10/23.
//

#include "CircularBuffer.h"
#include <memory>
#include <cstddef>
#include <cstdint>
namespace our_graph::utils {

CircularBuffer::CircularBuffer(size_t buffer_size) {
  data_ = InnerAlloc(buffer_size);
  size_ = buffer_size;
  tail_ = data_;
  head_ = data_;
}

CircularBuffer::~CircularBuffer() noexcept {
  InnerDealloc();
}

void *CircularBuffer::InnerAlloc(size_t size) noexcept {
  //todo:优化（如mmap）

  return ::malloc(2 * size);
}

void CircularBuffer::InnerDealloc() noexcept {
  //todo

  ::free(data_);

  data_ = nullptr;
}

void CircularBuffer::Circularize() noexcept {
  //todo:(mmap时)

  // 超出一个buffer范围时返回首位
  if ((intptr_t(head_) - intptr_t(data_)) > int32_t (size_)) {
    head_ = data_;
  }
  tail_ = head_;
}


}