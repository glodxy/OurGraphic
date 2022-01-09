//
// Created by Glodxy on 2022/1/8.
//

#include "Resource/include/RenderTarget.h"
#include "Resource/include/Texture.h"
#include "ResourceAllocator.h"
#include "Backend/include/DriverEnum.h"
#include "Backend/include/RenderTargetInfo.h"
namespace our_graph {

struct RenderTarget::Detail {
  Attachment attachments[ATTACHMENT_CNT] = {};
  uint32_t width;
  uint32_t height;
  uint8_t samples = 1;
  Driver* driver;
};

using BuilderType = RenderTarget;
BuilderType::Builder::Builder(Driver* driver) noexcept {
  impl_->driver = driver;
}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

RenderTarget::Builder &RenderTarget::Builder::WithTexture(AttachmentPoint attachment, Texture *texture) noexcept {
  impl_->attachments[size_t(attachment)].texture = texture;
  return *this;
}

RenderTarget::Builder &RenderTarget::Builder::MipLevel(AttachmentPoint attachment, uint8_t level) noexcept {
  impl_->attachments[size_t(attachment)].mip_level = level;
  return *this;
}

RenderTarget::Builder &RenderTarget::Builder::Face(AttachmentPoint attachment, CubemapFace face) noexcept {
  impl_->attachments[size_t(attachment)].face = face;
  return *this;
}

RenderTarget::Builder &RenderTarget::Builder::Layer(AttachmentPoint attachment, uint32_t layer) noexcept {
  impl_->attachments[size_t(attachment)].layer = layer;
  return *this;
}

RenderTarget* RenderTarget::Builder::Build() {
  // todo assert

  const uint8_t mip_level = impl_->attachments[0].mip_level;
  uint32_t min_width = std::numeric_limits<uint32_t>::max();
  uint32_t max_width = 0;
  uint32_t min_height = std::numeric_limits<uint32_t>::max();
  uint32_t max_height = 0;
  for (const auto& attachment : impl_->attachments) {
    if (attachment.texture) {
      const uint32_t w = attachment.texture->GetWidth(mip_level);
      const uint32_t h = attachment.texture->GetHeight(mip_level);
      min_width = std::min(min_width, w);
      min_height = std::min(min_height, h);
      max_width = std::max(max_width, w);
      max_height = std::max(max_height, h);
    }
  }

  if (min_width != max_width ||
      min_height != max_height) {
    LOG_ERROR("RenderTargetBuilder", "All Attachment's w/h Must Match");
    return nullptr;
  }

  impl_->width = min_width;
  impl_->height = max_height;
  return ResourceAllocator::Get().CreateRenderTarget(*this);
}

/*---------------------------------------*/
RenderTarget::RenderTarget(const Builder &builder) :
  driver_(builder->driver),
  supported_color_attachments_count_(ATTACHMENT_CNT) {

  std::copy(std::begin(builder->attachments), std::end(builder->attachments),
            std::begin(attachment_));

  MRT mrt;
  TargetBufferInfo depth_info = {};
  auto setAttachment = [this](TargetBufferInfo& info, AttachmentPoint attachmentPoint) {
    Attachment const& attachment = attachment_[(size_t)attachmentPoint];
    auto t = attachment.texture;
    info.handle_ = t->GetHandle();
    info.level_  = attachment.mip_level;
    if (t->GetSamplerType() == Texture::Sampler::SAMPLER_CUBEMAP) {
      info.face_ = attachment.face;
    } else {
      info.layer_ = attachment.layer;
    }
  };

  // 设置render target
  for (size_t i = 0; i< MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (attachment_[i].texture) {
      attachment_mask_ |= GetTargetBufferFlagsAtColor(i);
      setAttachment(mrt[i], static_cast<AttachmentPoint>(i));
    }
  }

  // 如果有设置depth，则也将该attachment设置
  if (attachment_[size_t(AttachmentPoint::DEPTH)].texture != nullptr) {
    attachment_mask_ |= TargetBufferFlags::DEPTH;
    setAttachment(depth_info, AttachmentPoint::DEPTH);
  }

  handle_ = driver_->CreateRenderTarget(attachment_mask_,
                                        builder->width,
                                        builder->height,
                                        builder->samples,
                                        mrt,
                                        depth_info,
                                        {});
}

RenderTarget::~RenderTarget() noexcept {}
void RenderTarget::Destroy() {driver_->DestroyRenderTarget(handle_);}

RenderTarget::Attachment RenderTarget::GetAttachment(AttachmentPoint attachment) const noexcept {
  return attachment_[size_t(attachment)];
}
TargetBufferFlags RenderTarget::GetAttachmentMask() const noexcept {
  return attachment_mask_;
}
uint8_t RenderTarget::GetSupportedColorAttachmentsCount() const noexcept {
  return supported_color_attachments_count_;
}




Texture *RenderTarget::GetTexture(AttachmentPoint attachment) const noexcept {
  return GetAttachment(attachment).texture;
}

uint8_t RenderTarget::GetMipLevel(AttachmentPoint attachment) const noexcept {
  return GetAttachment(attachment).mip_level;
}

RenderTarget::CubemapFace RenderTarget::GetFace(AttachmentPoint attachment) const noexcept {
  return GetAttachment(attachment).face;
}

uint32_t RenderTarget::GetLayer(AttachmentPoint attachment) const noexcept {
  return GetAttachment(attachment).layer;
}
}  // namespace our_graph