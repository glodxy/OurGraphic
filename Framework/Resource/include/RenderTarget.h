//
// Created by Glodxy on 2021/10/25.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_RENDERTARGET_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_RENDERTARGET_H_

#include "Backend/include/DriverEnum.h"
#include "Framework/Resource/base/BuilderBase.h"
#include "Framework/Resource/include_internal/ResourceBase.h"
#include "Backend/include/Driver.h"

namespace our_graph {

class Texture;

class RenderTarget : public ResourceBase {
  friend class ResourceAllocator;
  struct Detail;
 public:
  ~RenderTarget() override;
  void Destroy() override;
 public:
  using CubemapFace = TextureCubemapFace;

  struct Attachment {
    Texture* texture = nullptr;
    uint8_t mip_level = 0;
    CubemapFace face = CubemapFace::POSITIVE_X;
    uint32_t layer = 0;
  };

  static constexpr uint8_t MIN_SUPPORTED_COLOR_ATTACHMENTS_COUNT =
      MIN_SUPPORTED_RENDER_TARGET_COUNT;

  // 支持的最大输出数
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


    Builder& WithTexture(AttachmentPoint attachment,
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

  RenderTargetHandle GetHandle() const noexcept {return handle_;}

  Attachment GetAttachment(AttachmentPoint attachment) const noexcept;

  TargetBufferFlags GetAttachmentMask() const noexcept;

 protected:
  RenderTarget(const Builder& builder);

 private:
  static constexpr uint32_t ATTACHMENT_CNT = MAX_SUPPORTED_COLOR_ATTACHMENT_COUNT + 1;
  Attachment attachment_[ATTACHMENT_CNT];
  RenderTargetHandle handle_;
  TargetBufferFlags attachment_mask_;
  const uint8_t supported_color_attachments_count_;
  Driver* driver_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_RENDERTARGET_H_
