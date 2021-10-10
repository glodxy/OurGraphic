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
#include "VulkanDisposer.h"
#include "VulkanHandles.h"
#include "VulkanSamplerCache.h"

namespace our_graph {
class VulkanDriver : public DriverApi {
 public:
  void Init(std::unique_ptr<IPlatform> platform) override;

  void Clear() override;

  SwapChainHandle CreateSwapChain(void *native_window, uint64_t flags) override;
  void DestroySwapChain(SwapChainHandle handle) override;

  RenderTargetHandle CreateDefaultRenderTarget() override;
  void DestroyRenderTarget(RenderTargetHandle handle) override;

  // 随时间更新fence
  void Tick() override;

  void BeginFrame(int64_t time_ns, uint32_t frame_id) override {}
  void EndFrame(uint32_t frame_id) override;

  void BeginRenderPass(RenderTargetHandle handle, const RenderPassParams &params) override;
  // 设置当前交换链
  void MakeCurrent(SwapChainHandle draw, SwapChainHandle read) override;

  // 提交命令，并刷新交换链
  void Commit(SwapChainHandle handle) override;

  void Draw(PipelineState state, RenderPrimitiveHandle handle) override;
 private:
  void CreateEmptyTexture(VulkanStagePool& stage_pool);

  /**
   * 刷新交换链
   * 通常发生在窗口大小变更时
   * */
  void RefreshSwapChain();

  void SetupRasterState(const VulkanRenderTarget* rt,
                        RasterState raster_state,
                        PolygonOffset depth_offset);

  void SetupSamplers(VulkanShader* program);
  void SetupScissor(Viewport view_port,
                    const VulkanRenderTarget* rt,
                    VkCommandBuffer cmd_buffer);

  std::unique_ptr<VulkanInstance> instance_;
  std::unique_ptr<VulkanDevice> device_;
  std::unique_ptr<VulkanStagePool> stage_pool_;
  std::unique_ptr<VulkanFBOCache> fbo_cache_;
  std::unique_ptr<VulkanPipelineCache> pipeline_cache_;
  std::unique_ptr<VulkanSamplerCache> sampler_cache_;
  std::unique_ptr<IPlatform> platform_;
  std::unique_ptr<VulkanDisposer> disposer_;

  VulkanRenderTarget* current_render_target_ {nullptr};
  VulkanSamplerGroup* sampler_bindings_[VulkanPipelineCache::SAMPLER_BINDING_COUNT] = {};
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDRIVER_H_
