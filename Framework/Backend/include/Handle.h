//
// Created by chaytian on 2021/9/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_HANDLE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_HANDLE_H_

#include <cstdint>
#include <climits>
#include <numeric>

namespace our_graph {
/**
 * 该类不由外部持有
 * */
class HandleBase {
 public:
  using HandleId = uint32_t;
  // 空句柄
  static constexpr uint32_t NULL_HANDLE = HandleId { std::numeric_limits<HandleId>::max() };

  constexpr HandleBase() noexcept: object_(NULL_HANDLE) {}

  explicit HandleBase(HandleId id) noexcept : object_(id) {
    assert(object_ != NULL_HANDLE); // usually means an uninitialized handle is used
  }

  HandleBase(HandleBase const& rhs) noexcept = default;
  // 只能存在一个句柄，所以直接进行转移
  HandleBase(HandleBase&& rhs) noexcept : object_(rhs.object_) {
    rhs.object_ = NULL_HANDLE;
  }

  // 同理，直接进行转移，防止多个对象持有
  HandleBase& operator=(HandleBase const& rhs) noexcept = default;
  HandleBase& operator=(HandleBase&& rhs) noexcept {
    std::swap(object_, rhs.object_);
    return *this;
  }

  explicit operator bool() const noexcept { return object_ != NULL_HANDLE; }

  void Clear() noexcept { object_ = NULL_HANDLE; }

  bool operator==(const HandleBase& rhs) const noexcept { return object_ == rhs.object_; }
  bool operator!=(const HandleBase& rhs) const noexcept { return object_ != rhs.object_; }

  HandleId Get() const noexcept { return object_; }

 protected:
  HandleId object_;
};


/**
 * 该类暴露给外部使用
 * */
template <class T>
class Handle : public HandleBase {
  using HandleBase::HandleBase;
 public:
  template<class B, std::enable_if_t<std::is_base_of<T, B>::value>>
  Handle(const Handle<B>& base) : HandleBase(base) {}
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_HANDLE_H_
