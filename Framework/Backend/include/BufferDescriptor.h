//
// Created by Glodxy on 2021/10/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_BUFFERDESCRIPTOR_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_BUFFERDESCRIPTOR_H_
#include <functional>

namespace our_graph {

class BufferDescriptor {
 public:
  // 该回调用于销毁buffer资源（与Driver层无关的部分）
  using Callback = std::function<void(void* buffer, size_t size, void* user)>;

  BufferDescriptor() noexcept = default;
  ~BufferDescriptor() noexcept {
    if (callback_) {
      callback_(buffer_, size_, user_);
    }
  }

  /**
   * 禁用拷贝
   * */
  BufferDescriptor(const BufferDescriptor&) = delete;
  BufferDescriptor& operator=(const BufferDescriptor&) = delete;

  /**
   * buffer只允许一个对象持有
   * */
  BufferDescriptor(BufferDescriptor&& rhs) noexcept
                  : buffer_(rhs.buffer_), size_(rhs.size_),
                    callback_(rhs.callback_) {
    rhs.buffer_ = nullptr;
    rhs.callback_ = nullptr;
  }

  BufferDescriptor& operator=(BufferDescriptor&& rhs) noexcept {
    if (this != &rhs) {
      buffer_ = rhs.buffer_;
      size_ = rhs.size_;
      callback_ = rhs.callback_;
      user = rhs.user_;

      rhs.buffer_ = nullptr;
      rhs.callback_ = nullptr;
    }
  }

  BufferDescriptor(const void* buffer, size_t size,
                   Callback callback = nullptr, void* user = nullptr) noexcept
                   : buffer_(const_cast<void*>(buffer),
                     size_(size), callback_(callback)),
                     user_(user) {

  }

  void SetCallback(Callback callback, void* user = nullptr) noexcept {
    callback_ = callback;
    user_ = user;
  }

  bool HasCallback() const noexcept {return callback_ != nullptr;}

  Callback GetCallback() const noexcept {
    return callback_;
  }

  void* GetUser() const noexcept {
    return user_;
  }

  /**
   * cpu中的数据部分
   * */
  void* buffer_ {nullptr};

  /**
   * CPU部分的数据大小
   * */
  size_t size_ {0};
 private:
  Callback callback_ {nullptr};
  void* user_ {nullptr};
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_BUFFERDESCRIPTOR_H_
