//
// Created by Glodxy on 2021/10/23.
//

#ifndef OUR_GRAPHIC_UTILS_CIRCULARBUFFER_H_
#define OUR_GRAPHIC_UTILS_CIRCULARBUFFER_H_

namespace our_graph::utils {
class CircularBuffer {
 public:
  static constexpr size_t BLOCK_BITS = 12; // 每块12bit， 4kb大小
  static constexpr size_t BLOCK_SIZE = 1 << BLOCK_BITS;
  static constexpr size_t BLOCK_MASK = BLOCK_SIZE - 1;

  /**
   * 该size最好是3*required_size，以备3个线程的需要：render/UI/common
   * */
  explicit CircularBuffer(size_t buffer_size);

  CircularBuffer(CircularBuffer const& rhs) = delete;
  CircularBuffer(CircularBuffer&& rhs) noexcept = delete;
  CircularBuffer& operator=(CircularBuffer const& rhs) = delete;
  CircularBuffer& operator=(CircularBuffer&& rhs) noexcept = delete;

  ~CircularBuffer() noexcept;

  inline void* Allocate(size_t size) noexcept {
     char* const cur =static_cast<char*>(head_);
    head_ = cur + size;
    return cur;
  }

  // 该循环buffer的总size
  size_t Size() const noexcept {
    return size_;
  }

  bool Empty() const noexcept {
    return head_ == tail_;
  }

  void* GetHead() const noexcept {
    return head_;
  }

  void* GetTail()const noexcept {
    return tail_;
  }

  /**
   * 每次调用GetRequiredSize后都需要调用该函数
   * 该函数的作用为调整head的位置防止其超出buffer的范围
   * 如果head超出范围则移至首位
   * */
  void Circularize() noexcept;

 private:
  void* InnerAlloc(size_t size) noexcept;
  void InnerDealloc() noexcept;

  void* data_ = nullptr;
  size_t size_ = 0;

  void* tail_ = nullptr;
  void* head_ = nullptr;
};
}  // namespace our_graph::utils
#endif //OUR_GRAPHIC_UTILS_CIRCULARBUFFER_H_
