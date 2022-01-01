//
// Created by Glodxy on 2021/12/7.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMBUFFER_H_
#include <cstddef>
#include <memory>
#include <type_traits>
#include "Utils/Math/Math.h"
#include "Framework/Backend/include/Driver.h"
namespace our_graph {
class UniformBuffer {
 public:
  UniformBuffer() noexcept = default;
  explicit UniformBuffer(size_t size) noexcept;

  UniformBuffer(const UniformBuffer&) = delete;
  UniformBuffer& operator=(const UniformBuffer&) = delete;

  // 可以使用std::move
  UniformBuffer(UniformBuffer&& r) noexcept;
  UniformBuffer& operator=(UniformBuffer&& r) noexcept;

  ~UniformBuffer() noexcept;

  UniformBuffer& SetUniforms(const UniformBuffer& r) noexcept;

  // 禁用某一段buffer，并返回新的首部指针
  void* InvalidateUniforms(size_t offset, size_t size) noexcept {
    if (offset + size > size_) {
      LOG_ERROR("Uniform", "Invalidate Error! offset:{}, size:{}, current-size:{}",
                offset, size, size_);
      assert(offset + size <= size_);
    }
    dirty_ = true;
    return static_cast<char*>(buffer_) + offset;
  }
  void* Invalidate() noexcept {
    return InvalidateUniforms(0, size_);
  }


  const void* GetBuffer() const noexcept {return buffer_;}
  size_t GetSize() const noexcept {return size_;}
  bool IsDirty() const noexcept {return dirty_;}
  // 清除修改标记
  void CleanFlag() const noexcept {dirty_ = false;}

 public:
  template <typename T>
  using is_supported_type = typename std::enable_if<
        std::is_same<float, T>::value ||
            std::is_same<int32_t, T>::value ||
            std::is_same<uint32_t, T>::value ||
            std::is_same<math::Vec2i, T>::value ||
            std::is_same<math::Vec3i, T>::value ||
            std::is_same<math::Vec4i, T>::value ||
            std::is_same<math::Vec2u, T>::value ||
            std::is_same<math::Vec3u, T>::value ||
            std::is_same<math::Vec4u, T>::value ||
            std::is_same<math::Vec2, T>::value ||
            std::is_same<math::Vec3, T>::value ||
            std::is_same<math::Vec4, T>::value ||
            std::is_same<math::Mat3, T>::value ||
            std::is_same<math::Mat4, T>::value>::type;

  template<class T, typename = is_supported_type<T>>
  void SetUniformArray(size_t offset, const T* src, size_t cnt) noexcept {
    SetUniformArrayUntyped<sizeof(T)>(offset, src, cnt);
  }

  template<class T, typename = is_supported_type<T>>
  inline void SetUniform(size_t offset, const T& v) noexcept {
    SetUniformUntyped<sizeof(T)>(offset, &v);
  }

  template<class T, typename = is_supported_type<T>>
  T GetUniform(size_t offset) const noexcept {
    return *reinterpret_cast<const T*>(static_cast<const char*>(buffer_) + offset);
  }


  template<size_t S>
  void SetUniformUntyped(size_t offset, const void* v) noexcept;

  template<size_t S>
  void SetUniformArrayUntyped(size_t offset, const void* v, size_t cnt) noexcept;


  template<class T, typename = is_supported_type<T>>
  static inline void SetUniform(void* addr, size_t offset, const T& value) noexcept {
    SetUniformUntyped<sizeof(T)>(addr, offset, &value);
  }

  BufferDescriptor ToBufferDescriptor(Driver* driver) const noexcept{
    return ToBufferDescriptor(driver, 0, GetSize());
  }

  BufferDescriptor ToBufferDescriptor(Driver* driver,
                                      size_t offset, size_t size) const noexcept {
    BufferDescriptor p;
    p.size_ = size_;
    p.buffer_ = driver->Allocate(p.size_);
    memcpy(p.buffer_, static_cast<const char*>(GetBuffer()) + offset, p.size_);
    CleanFlag();
    return p;
  }

 private:
  template<size_t S, std::enable_if_t<
      S == 4 || S == 8 || S == 12 || S == 16 || S == 36 || S == 64, bool> = true>
  static void SetUniformUntyped(void* addr, size_t offset, const void* value) noexcept {
    memcpy(static_cast<char*>(addr) + offset, value, S);
  }

  bool IsLocalStorage() const noexcept {
    return buffer_ == storage_;
  }
 private:
  // 96byte的本地存储
  // 多余部分需要自行分配
  char storage_[96];
  // 实际的数据位置，可能位于本地
  void* buffer_ = nullptr;
  // 数据的大小，byte
  uint32_t size_ = 0;
  // 脏位，判断是否有变更
  mutable bool dirty_ = false;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMBUFFER_H_
