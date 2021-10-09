//
// Created by Glodxy on 2021/10/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANHANDLES_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANHANDLES_H_

#include "../include_internal/IResource.h"
#include "../include_internal/DriverEnum.h"
#include "VulkanContext.h"
#include "VulkanTexture.h"

namespace our_graph {

class VulkanRenderTarget : private IRenderTarget {
 public:
  VulkanRenderTarget(uint32_t width, uint32_t height, uint8_t samples,
                     VulkanAttachment color[MAX_SUPPORTED_RENDER_TARGET_COUNT],
                     VulkanAttachment depth_stencil[2],
                     VulkanStagePool& stage_pool);

  explicit VulkanRenderTarget();

  void TransformClientRectToPlatform(VulkanSwapChain* current_surface, VkRect2D* bounds) const;
  void TransformClientRectToPlatform(VulkanSwapChain* current_surface, VkViewport* bounds) const;

  VkExtent2D GetExtent(VulkanSwapChain* current_surface) const;
  VulkanAttachment GetColor(VulkanSwapChain* current_surface, int target) const;
  VulkanAttachment GetMsaaColor(int target) const;
  VulkanAttachment GetDepth(VulkanSwapChain* current_surface) const;
  VulkanAttachment GetMsaaDepth() const;

  int GetColorTargetCount(const VulkanRenderPass& pass) const;
  uint8_t GetSamples() const {return samples_;}
  bool HasDepth() const {return depth_.format != VK_FORMAT_UNDEFINED;}
  bool IsSwapChain() const {return !offscreen_;}

 private:
  VulkanAttachment color_[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  VulkanAttachment depth_ = {};
  VulkanAttachment msaa_attachments_[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  VulkanAttachment msaa_depth_attachment_ = {};
  const bool offscreen_ : 1; // 是否有用于显示： 1:未使用
  uint8_t samples_ : 7;  // 采样数
};

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANHANDLES_H_
