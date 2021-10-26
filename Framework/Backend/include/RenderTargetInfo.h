//
// Created by Glodxy on 2021/10/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_RENDERTARGETINFO_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_RENDERTARGETINFO_H_

#include "DriverEnum.h"
#include "Handle.h"

namespace our_graph {
/**
 * 该类用于描述rendertarget的信息
 * 包括绑定的texture， mipmap的level， 层级/是否是3d
 * */
class TargetBufferInfo {
 public:
  TargetBufferInfo(TextureHandle handle, uint8_t level = 0) noexcept
    : handle_(handle), level_(level) {}

  TargetBufferInfo(TextureHandle handle, uint8_t level, TextureCubemapFace face) noexcept
    : handle_(handle), level_(level), face_(face) {}

  TargetBufferInfo(TextureHandle handle, uint8_t level, uint16_t layer) noexcept
    : handle_(handle), level_(level), layer_(layer) {}

  explicit TargetBufferInfo(TextureCubemapFace face) noexcept : face_(face) {}

  explicit TargetBufferInfo(uint16_t layer) noexcept : layer_(layer) {}

  TextureHandle handle_;
  uint8_t level_ = 0;
  union {
    // cubemap
    TextureCubemapFace face_;
    // 3d texture
    uint16_t layer_ = 0;
  };
  TargetBufferInfo() noexcept {}

};

/**
 * 该类是多渲染目标
 * */
class MRT {
 public:
  const TargetBufferInfo& operator[](size_t i) const noexcept {
    return infos_[i];
  }

  TargetBufferInfo& operator[](size_t i) noexcept {
    return infos_[i];
  }

  MRT() noexcept = default;

  MRT(const TargetBufferInfo& color) noexcept
    : infos_{color} {}

  MRT(const TargetBufferInfo& color0, const TargetBufferInfo& color1) noexcept
    : infos_{color0, color1} {}

  MRT(const TargetBufferInfo& color0, const TargetBufferInfo& color1,
      const TargetBufferInfo& color2) noexcept
     : infos_{color0, color1, color2} {}

  MRT(const TargetBufferInfo& color0, const TargetBufferInfo& color1,
      const TargetBufferInfo& color2, const TargetBufferInfo& color3) noexcept
      : infos_{color0, color1, color2, color3} {}

  MRT(TextureHandle handle, uint8_t level, uint16_t layer) noexcept
      : infos_{{handle, level, layer}} {}
 private:
  TargetBufferInfo infos_[MAX_SUPPORTED_RENDER_TARGET_COUNT];
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_RENDERTARGETINFO_H_
