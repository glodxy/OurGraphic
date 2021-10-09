//
// Created by Glodxy on 2021/10/9.
//
#include "VulkanDef.h"
#include "VulkanHandles.h"
#include "VulkanSwapChain.h"
#include <utility>


namespace our_graph {

static void FlipVertically(VkRect2D* rect, uint32_t framebuffer_height) {
  // 底点减去总高度
  rect->offset.y = framebuffer_height - rect->offset.y - rect->extent.height;
}

static void FlipVertically(VkViewport* rect, uint32_t framebuffer_height) {
  rect->y = framebuffer_height - rect->y - rect->height;
}

/**
 * 将rect放缩至framebuffer范围内
 * */
static void ClampToFrameBuffer(VkRect2D* rect, uint32_t fb_width, uint32_t fb_height) {
  int32_t x = std::max(rect->offset.x, 0);
  int32_t y = std::max(rect->offset.y, 0);
  int32_t right = std::min(rect->offset.x + (int32_t)rect->extent.width, (int32_t) fb_width);
  int32_t top = std::min(rect->offset.y + (int32_t)rect->extent.height, (int32_t)fb_height);
  rect->offset.x = std::min(x, (int32_t)fb_width);
  rect->offset.y = std::min(y, (int32_t)fb_height);
  rect->extent.width = std::max(right - x, 0);
  rect->extent.height = std::max(top - y, 0);
}

/**
 * 根据已有attachment拷贝attachment
 * */
static VulkanAttachment CreateAttachment(VulkanAttachment spec) {
  if (spec.texture == nullptr) {
    return spec;
  }
  return {
    .format = spec.texture->GetVKFormat(),
    .image = spec.texture->GetVKImage(),
    .view = {},
    .memory = {},
    .texture = spec.texture,
    .layout = VulkanUtils::GetTextureLayout(spec.texture->usage_),
    .level = spec.level,
    .layer = spec.layer
  };
}

VulkanRenderTarget::VulkanRenderTarget() : IRenderTarget(0, 0),
    offscreen_(false), samples_(1) {

}

/**
 * @param depth_stencil: [0]:depth, [1]:stencil
 * */
VulkanRenderTarget::VulkanRenderTarget(uint32_t width,
                                       uint32_t height,
                                       uint8_t samples,
                                       VulkanAttachment *color,
                                       VulkanAttachment *depth_stencil,
                                       VulkanStagePool &stage_pool) :
    IRenderTarget(width, height), offscreen_(true), samples_(samples) {

  /**
   * 对每一个attachment创建相应的view
   * */
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    const VulkanAttachment& spec = color[i];
    color_[i] = CreateAttachment(spec);
    VulkanTexture* texture = spec.texture;
    if (!texture) {
      continue;
    }
    color_[i].view = texture->GetAttachmentView(spec.level, spec.layer,
                                                VK_IMAGE_ASPECT_COLOR_BIT);
  }

  const VulkanAttachment& depth_spec = depth_stencil[0];
  depth_ = CreateAttachment(depth_spec);
  VulkanTexture* depth_texture = depth_.texture;
  if (depth_texture) {
    depth_.view = depth_texture->GetAttachmentView(depth_.level, depth_.layer,
                                                   VK_IMAGE_ASPECT_DEPTH_BIT);
  }

  if (samples == 1) {
    // 采样数为1，不管
    return;
  }

  const auto& limits = VulkanContext::Get().physical_device_properties_.limits;
  samples_ = samples =
      VulkanUtils::ReduceSampleCount(
          samples,
          limits.framebufferDepthSampleCounts & limits.framebufferColorSampleCounts);

  const int level = 1;
  const int depth = 1;

  // 创建msaa纹理
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    const VulkanAttachment& spec = color[i];
    VulkanTexture* texture = spec.texture;
    // 采样数为1，需要手动创建
    if (texture && texture->samples_ == 1) {
      VulkanTexture* ms_texture = texture->GetMSAA();
      if (ms_texture) {
        ms_texture = new VulkanTexture(texture->sampler_type_, level,
                                       texture->format_, samples, width, height,
                                       depth, texture->usage_, stage_pool);
        texture->SetMSAA(ms_texture);
      }
      msaa_attachments_[i] = CreateAttachment({.texture = ms_texture});
      msaa_attachments_[i].view = ms_texture->GetAttachmentView(0, 0,
                                                                VK_IMAGE_ASPECT_COLOR_BIT);
    }
    if (texture && texture->samples_ > 1) {
      msaa_attachments_[i] = color_[i];
    }
  }

  if (!depth_texture) {
    return;
  }

  if (depth_texture->samples_ > 1) {
    msaa_depth_attachment_ = depth_;
    return;
  }

  // 否则创建深度纹理的msaa
  VulkanTexture* depth_ms_texture = depth_texture->GetMSAA();
  if (!depth_ms_texture) {
    depth_ms_texture = new VulkanTexture(depth_texture->sampler_type_, level,
                                         depth_texture->format_, samples, width, height,
                                         depth, depth_texture->usage_, stage_pool);
    depth_texture->SetMSAA(depth_ms_texture);
  }

  msaa_depth_attachment_ = CreateAttachment(
      {.texture = depth_ms_texture,
       .layout = {},
       .level = depth_spec.level,
       .layer = depth_spec.layer});
  msaa_depth_attachment_.view = depth_ms_texture->GetAttachmentView(
      depth_spec.level, depth_spec.layer, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRenderTarget::TransformClientRectToPlatform(VulkanSwapChain *current_surface, VkRect2D *bounds) const {
  const auto& extent = GetExtent(current_surface);
  FlipVertically(bounds, extent.height);
  ClampToFrameBuffer(bounds, extent.width, extent.height);
}

void VulkanRenderTarget::TransformClientRectToPlatform(VulkanSwapChain *current_surface, VkViewport *bounds) const {
  FlipVertically(bounds, GetExtent(current_surface).height);
}

VkExtent2D VulkanRenderTarget::GetExtent(VulkanSwapChain *current_surface) const {
  if (offscreen_) {
    return {width_, height_};
  }

  return current_surface->GetClientSize();
}

VulkanAttachment VulkanRenderTarget::GetColor(VulkanSwapChain *current_surface, int target) const {
  return (offscreen_ || target > 0) ? color_[target] : current_surface->GetColor();
}

VulkanAttachment VulkanRenderTarget::GetMsaaColor(int target) const {
  return msaa_attachments_[target];
}

VulkanAttachment VulkanRenderTarget::GetMsaaDepth() const {
  return msaa_depth_attachment_;
}

int VulkanRenderTarget::GetColorTargetCount(const VulkanRenderPass &pass) const {
  if (!offscreen_) {
    return 1;
  }
  int cnt = 0;
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (color_[i].format == VK_FORMAT_UNDEFINED) {
      continue;
    }
    // 判断当前pass有没有在使用
    if (!(pass.subpassMask & (1 << i)) || pass.currentSubpass == 1) {
      cnt++;
    }
  }
  return cnt;
}

}  // namespace our_graph