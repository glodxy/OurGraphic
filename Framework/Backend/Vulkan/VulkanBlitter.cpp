//
// Created by Glodxy on 2021/12/1.
//

#include "VulkanBlitter.h"
#include "VulkanHandles.h"
#include "VulkanSwapChain.h"
namespace our_graph {

static VulkanLayoutTransition GetTransitionLayout(VulkanLayoutTransition transition) {
  switch (transition.newLayout) {
    // 被shader使用时，切换为读取状态
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_GENERAL:
    {
      transition.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      transition.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      transition.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      transition.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      break;
    }

    // 被用作render target或用于展示
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    {
      transition.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      transition.dstAccessMask = 0;
      transition.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      transition.dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      break;
    }
  }
  return transition;
}

VulkanBlitter::VulkanBlitter(VulkanStagePool &pool,
                             VulkanPipelineCache &pipeline_cache,
                             VulkanFBOCache &fbo_cache,
                             VulkanSamplerCache &sampler_cache):
                             stage_pool_(pool),
                             pipeline_cache_(pipeline_cache),
                             fbo_cache_(fbo_cache),
                             sampler_cache_(sampler_cache) {

}

void VulkanBlitter::BlitColor(Args arg) {
  const VulkanAttachment src = arg.src_target->GetColor(VulkanContext::Get().current_surface_, arg.target_idx);
  const VulkanAttachment dst = arg.dst_target->GetColor(VulkanContext::Get().current_surface_, 0);
  const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;

  BlitInGPU(aspect, arg.filter, arg.src_target->GetExtent(VulkanContext::Get().current_surface_),
            src, dst, arg.src_rect, arg.dst_rect);
}

void VulkanBlitter::BlitDepth(Args arg) {
  VulkanSwapChain* const sc = VulkanContext::Get().current_surface_;
  const VulkanAttachment src = arg.src_target->GetDepth(sc);
  const VulkanAttachment dst = arg.src_target->GetDepth(sc);
  const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

  if (src.texture && src.texture->samples_ > 1 && dst.texture && dst.texture->samples_ == 1) {
    LOG_ERROR("VulkanBlitter", "depth not supported resolve! from {} to {}",
              src.texture->samples_, dst.texture->samples_);
    return;
  }

  BlitInGPU(aspect, arg.filter, arg.src_target->GetExtent(sc), src, dst, arg.src_rect,
           arg.dst_rect);
}

void VulkanBlitter::BlitInGPU(VkImageAspectFlags aspect,
                              VkFilter filter,
                              const VkExtent2D src_extent,
                              VulkanAttachment src,
                              VulkanAttachment dst,
                              const VkOffset3D *src_rect,
                              const VkOffset3D *dst_rect) {
  const VkImageBlit blit_region[1] = {{
    .srcSubresource = {aspect, src.level, src.layer, 1},
    .srcOffsets = {src_rect[0], src_rect[1]},
    .dstSubresource = {aspect, dst.level, dst.layer, 1},
    .dstOffsets = {dst_rect[0], dst_rect[1]}
  }};

  const VkImageResolve resolve_region[1] = {{
    .srcSubresource = {aspect, src.level, src.layer, 1},
    .srcOffset = src_rect[0],
    .dstSubresource = {aspect, dst.level, dst.layer, 1},
    .dstOffset = dst_rect[0]
  }};

  const VkImageSubresourceRange src_range = {
      .aspectMask = aspect,
      .baseMipLevel = src.level,
      .levelCount = 1,
      .baseArrayLayer = src.layer,
      .layerCount = 1
  };

  const VkImageSubresourceRange dst_range = {
      .aspectMask = aspect,
      .baseMipLevel = dst.level,
      .levelCount = 1,
      .baseArrayLayer = dst.layer,
      .layerCount = 1
  };

  const VkCommandBuffer cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;
  // 将src image切换至传输read状态
  VulkanUtils::TransitionImageLayout(cmd_buffer, {
      src.image,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      src_range,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT
  });
  // 将dst image切换至传输写状态
  VulkanUtils::TransitionImageLayout(cmd_buffer, {
      dst.image,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      dst_range,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
  });

  // 当dst 采样数为1，src当采样数大于1时，说明需要解析提高分辨率
  if (src.texture && src.texture->samples_ > 1 && dst.texture && dst.texture->samples_ == 1) {
    if (aspect == VK_IMAGE_ASPECT_DEPTH_BIT) {
      LOG_ERROR("VulkanBlit", "BlitInGPU Not Supported Depth resolve!");
      return;
    }
    vkCmdResolveImage(cmd_buffer, src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.image,
                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, resolve_region);
  } else {
    // 否则直接调用blit
    vkCmdBlitImage(cmd_buffer, src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, blit_region, filter);
  }


  // 设置src image的状态
  if (src.texture) {
    VulkanUtils::TransitionImageLayout(cmd_buffer, GetTransitionLayout({
      .image = src.image,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VulkanUtils::GetTextureLayout(src.texture->usage_),
      .subresources = src_range
    }));
  } else {
    VulkanUtils::TransitionImageLayout(cmd_buffer, GetTransitionLayout({
      .image = src.image,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .subresources = src_range
    }));
  }

  // 获取目标纹理的layout
  // 如果本来存在则按照其用途生成布局
  // 否则认为会作为展示
  const VkImageLayout desired_layout = dst.texture ?
      VulkanUtils::GetTextureLayout(dst.texture->usage_) :
      VulkanContext::Get().current_surface_->GetColor().layout;

  VulkanUtils::TransitionImageLayout(cmd_buffer, GetTransitionLayout({
    .image = dst.image,
    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .newLayout = desired_layout,
    .subresources = dst_range
  }));
}

}