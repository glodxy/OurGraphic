//
// Created by Glodxy on 2021/10/14.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BASE_BUILDERBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_BASE_BUILDERBASE_H_
#include <utility>
#include <cstddef>

namespace our_graph {

template <typename T>
class BuilderBase {
 public:
  template<typename ... ARGS>
  explicit BuilderBase(ARGS&& ...) noexcept : impl_(new T(std::forward<ARGS>(args)...)) {

  }

  BuilderBase() noexcept : impl_(new T) {

  }

  ~BuilderBase() noexcept {
    delete impl_;
  }

  BuilderBase(BuilderBase const& rhs) noexcept : impl_(new T(*rhs.impl_)) {

  }

  BuilderBase& operator = (BuilderBase const& rhs) noexcept {
    *impl_ = *rhs.impl_;
    return *this;
  }

  // move ctor and copy operator can be implemented inline and don't need to be exported
  BuilderBase(BuilderBase&& rhs) noexcept : mImpl(rhs.mImpl) { rhs.mImpl = nullptr; }
  BuilderBase& operator = (BuilderBase&& rhs) noexcept {
    auto temp = mImpl;
    mImpl = rhs.mImpl;
    rhs.mImpl = temp;
    return *this;
  }

 protected:
  T* impl_ = nullptr;
  inline T* operator->() noexcept { return impl_; }
  inline T const* operator->() const noexcept { return impl_; }
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BASE_BUILDERBASE_H_
