//
// Created by Glodxy on 2021/12/1.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBLITTER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBLITTER_H_
#include "VulkanContext.h"
#include "VulkanDef.h"
#include "VulkanFBOCache.h"
#include "VulkanPipelineCache.h"
#include "VulkanSamplerCache.h"
#include "VulkanStagePool.h"
namespace our_graph {
class VulkanRenderTarget;

class VulkanBlitter {
 public:
  VulkanBlitter(VulkanStagePool& pool, VulkanPipelineCache& pipeline_cache,
                VulkanFBOCache& fbo_cache, VulkanSamplerCache& sampler_cache);

  struct Args {
    const VulkanRenderTarget* dst_target;
    const VkOffset3D* dst_rect;
    const VulkanRenderTarget* src_target;
    const VkOffset3D* src_rect;
    VkFilter filter = VK_FILTER_NEAREST;
    // todo:(unknown)
    int target_idx = 0;
  };

  void BlitColor(Args arg);
  void BlitDepth(Args arg);

 private:
  void BlitInGPU(VkImageAspectFlags aspect, VkFilter filter, const VkExtent2D src_extent,
                      VulkanAttachment src, VulkanAttachment dst, const VkOffset3D src_rect[2],
                      const VkOffset3D dst_rect[2]);

  VulkanStagePool &stage_pool_;
  VulkanPipelineCache& pipeline_cache_;
  VulkanFBOCache& fbo_cache_;
  VulkanSamplerCache& sampler_cache_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBLITTER_H_
