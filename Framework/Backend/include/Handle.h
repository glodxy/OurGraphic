//
// Created by chaytian on 2021/9/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_HANDLE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_HANDLE_H_

#include <cstdint>
#include <climits>
#include <numeric>
#include <memory>
#include <cassert>

namespace our_graph {
class IBufferObject;
class IIndexBuffer;
class IShader;
class IRenderPrimitive;
class IRenderTarget;
class ISamplerGroup;
class ISwapChain;
class ITexture;
class ITimerQuery;
class IVertexBuffer;

/**
 * 该类不由外部持有
 * */
class HandleBase {
  friend class HandleAllocator;
 public:
  using HandleId = uint32_t;

  static constexpr const uint32_t NULL_HANDLE = HandleId { std::numeric_limits<HandleId>::max() };

  constexpr HandleBase() noexcept : object_(NULL_HANDLE) {}

  explicit HandleBase(HandleId id) noexcept : object_(id) {
    assert(object_ != NULL_HANDLE); // usually means an uninitialized handle is used
  }

  HandleBase(HandleBase const & rhs) noexcept = default;

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

  HandleId GetId() const noexcept { return object_; }

 protected:
  mutable HandleId object_;
};


/**
 * 该类暴露给外部使用
 * */
template <class T>
class Handle : public HandleBase {
  using HandleBase::HandleBase;
 public:
  template<typename B, typename = std::enable_if_t<std::is_base_of<T, B>::value> >
  Handle(const Handle<B>& base) : HandleBase(base) {}

//  Handle(std::unique_ptr<T> r) {
//    object_ = r;
//  }
//
//  const T* Get() {
//    return object_.get();
//  }
// private:
//  std::unique_ptr<T> object_;
};

using BufferObjectHandle = Handle<IBufferObject>;
using IndexBufferHandle = Handle<IIndexBuffer>;
using ShaderHandle = Handle<IShader>;
using RenderPrimitiveHandle = Handle<IRenderPrimitive>;
using RenderTargetHandle = Handle<IRenderTarget>;
using SamplerGroupHandle = Handle<ISamplerGroup>;
using SwapChainHandle = Handle<ISwapChain>;
using TextureHandle = Handle<ITexture>;
using TimerQueryHandle = Handle<ITimerQuery>;
using VertexBufferHandle = Handle<IVertexBuffer>;
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_HANDLE_H_
