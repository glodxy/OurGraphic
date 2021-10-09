//
// Created by Glodxy on 2021/10/7.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDRIVER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDRIVER_H_

#include "../include/Driver.h"
#include "VulkanFBOCache.h"
#include "VulkanPipelineCache.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanStagePool.h"

namespace our_graph {
class VulkanDriver : public DriverApi {
 public:
  void Init(std::unique_ptr<IPlatform> platform) override;

  void Clear() override;

  SwapChainHandle CreateSwapChain(void *native_window, uint64_t flags) override;
  void DestroySwapChain(SwapChainHandle handle) override;

  RenderTargetHandle CreateDefaultRenderTarget() override;
  void DestroyRenderTarget(RenderTargetHandle handle) override;
 private:
  void CreateEmptyTexture(VulkanStagePool& stage_pool);
  std::unique_ptr<VulkanInstance> instance_;
  std::unique_ptr<VulkanDevice> device_;
  std::unique_ptr<VulkanStagePool> stage_pool_;
  std::unique_ptr<VulkanFBOCache> fbo_cache_;
  std::unique_ptr<VulkanPipelineCache> pipeline_cache_;
  std::unique_ptr<IPlatform> platform_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDRIVER_H_
