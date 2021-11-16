//
// Created by Glodxy on 2021/10/25.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_RENDERTARGET_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_RENDERTARGET_H_

#include "Backend/include/DriverEnum.h"
#include "Framework/Resource/base/BuilderBase.h"

#include "Backend/include/Driver.h"

namespace our_graph {

class Texture;

class RenderTarget {
  friend class ResourceAllocator;
  struct Detail;

 public:
  using CubemapFace = TextureCubemapFace;

  static constexpr uint8_t MIN_SUPPORTED_COLOR_ATTACHMENTS_COUNT =
      MIN_SUPPORTED_RENDER_TARGET_COUNT;

  static constexpr uint8_t MAX_SUPPORTED_COLOR_ATTACHMENT_COUNT =
      MAX_SUPPORTED_RENDER_TARGET_COUNT;

  // RenderTarget的绑定类别
  // 所有的attachment都会存在一个cubemap上以节省内存
  enum class AttachmentPoint : uint8_t {
    COLOR0 = 0,
    COLOR1 = 1,
    COLOR2 = 2,
    COLOR3 = 3,
    COLOR4 = 4,
    COLOR5 = 5,
    COLOR6 = 6,
    COLOR7 = 7,
    DEPTH = MAX_SUPPORTED_COLOR_ATTACHMENT_COUNT,
    COLOR = COLOR0,
  };

  class Builder : public BuilderBase<Detail> {
    friend class RenderTarget;

   public:
    Builder(Driver*) noexcept;
    Builder(Builder const& rhs) noexcept;
    Builder(Builder&& rhs) noexcept;
    ~Builder() noexcept;
    Builder& operator=(Builder const& rhs) noexcept;
    Builder& operator=(Builder&& rhs) noexcept;


    Builder& Texture(AttachmentPoint attachment,
                     Texture* texture) noexcept;
    // 设置mipmap的等级
    Builder& MipLevel(AttachmentPoint attachment,
                      uint8_t level) noexcept;
    // 设置attachment对应的face
    Builder& Face(AttachmentPoint attachment,
                  CubemapFace face) noexcept;

    Builder& Layer(AttachmentPoint attachment,
                   uint32_t layer) noexcept;

    RenderTarget* Build();
  };

  Texture* GetTexture(AttachmentPoint attachment) const noexcept;

  uint8_t GetMipLevel(AttachmentPoint attachment) const noexcept;

  CubemapFace GetFace(AttachmentPoint attachment) const noexcept;

  uint32_t GetLayer(AttachmentPoint attachment) const noexcept;

  uint8_t GetSupportedColorAttachmentsCount() const noexcept;
};

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_RENDERTARGET_H_
