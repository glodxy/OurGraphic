//
// Created by Glodxy on 2021/10/7.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDRIVER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDRIVER_H_

#include "../include/DriverApi.h"
#include "VulkanFBOCache.h"
#include "VulkanPipelineCache.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanStagePool.h"
#include "VulkanDisposer.h"
#include "VulkanHandles.h"
#include "VulkanSamplerCache.h"
#include "VulkanBlitter.h"


namespace our_graph {
class VulkanDriver : public DriverApi {
 public:
  VulkanDriver() noexcept;
  ~VulkanDriver();



  void Init(std::unique_ptr<IPlatform> platform) override;

  void Clear() override;

  SwapChainHandle CreateSwapChainS() override;
  void CreateSwapChainR(SwapChainHandle handle, void *native_window, uint64_t flags) override;
  void DestroySwapChain(SwapChainHandle handle) override;

  RenderTargetHandle CreateDefaultRenderTargetS() override;
  void CreateDefaultRenderTargetR(RenderTargetHandle handle) override;
  void DestroyRenderTarget(RenderTargetHandle handle) override;

  // 随时间更新fence
  void Tick() override;

  void BeginFrame(int64_t time_ns, uint32_t frame_id) override {}
  void EndFrame(uint32_t frame_id) override;

  void BeginRenderPass(RenderTargetHandle handle, const RenderPassParams &params) override;
  void EndRenderPass() override;
  void NextSubPass() override;

  void Flush() override;
  void Finish() override;

  RenderTargetHandle CreateRenderTargetS() override;
  void CreateRenderTargetR(RenderTargetHandle handle, TargetBufferFlags target_flags, uint32_t width, uint32_t height, uint8_t samples, MRT color, TargetBufferInfo depth, TargetBufferInfo stencil) override;

  RenderPrimitiveHandle CreateRenderPrimitiveS() override;
  void CreateRenderPrimitiveR(RenderPrimitiveHandle handle) override;
  void DestroyRenderPrimitive(RenderPrimitiveHandle handle) override;
  void SetRenderPrimitiveBuffer(RenderPrimitiveHandle handle, VertexBufferHandle vertex, IndexBufferHandle index) override;
  void SetRenderPrimitiveRange(RenderPrimitiveHandle handle, PrimitiveType type, uint32_t offset, uint32_t min_idx, uint32_t max_idx, uint32_t cnt) override;

  VertexBufferHandle CreateVertexBufferS() override;
  void CreateVertexBufferR(VertexBufferHandle handle, uint8_t buffer_cnt, uint8_t attribute_cnt, uint32_t vertex_cnt, AttributeArray attributes) override;
  void DestroyVertexBuffer(VertexBufferHandle handle) override;

  BufferObjectHandle CreateBufferObjectS() override;
  void CreateBufferObjectR(BufferObjectHandle handle, uint32_t bytes, BufferObjectBinding binding_type, BufferUsage usage) override;
  void DestroyBufferObject(BufferObjectHandle handle) override;

  void UpdateBufferObject(BufferObjectHandle handle, BufferDescriptor &&data, uint32_t byte_offset) override;
  void SetVertexBufferObject(VertexBufferHandle handle, uint32_t index, BufferObjectHandle buffer_handle) override;

  IndexBufferHandle CreateIndexBufferS() override;
  void CreateIndexBufferR(IndexBufferHandle handle, ElementType element_type, uint32_t index_cnt, BufferUsage usage) override;
  void DestroyIndexBuffer(IndexBufferHandle handle) override;
  void UpdateIndexBuffer(IndexBufferHandle handle, BufferDescriptor &&data, uint32_t byte_offset) override;

  void BindUniformBuffer(uint32_t idx, BufferObjectHandle handle) override;
  void BindUniformBufferRange(uint32_t idx, BufferObjectHandle handle, uint32_t offset, uint32_t size) override;

  SamplerGroupHandle CreateSamplerGroupS() override;
  void CreateSamplerGroupR(SamplerGroupHandle handle, uint32_t size) override;
  void BindSamplers(uint32_t idx, SamplerGroupHandle handle) override;
  void UpdateSamplerGroup(SamplerGroupHandle handle, SamplerGroup &&sampler_group) override;

  ShaderHandle CreateShaderS() override;
  void CreateShaderR(ShaderHandle handle, Program &&shaders) override;
  void DestroyShader(ShaderHandle handle) override;

  TextureHandle CreateTextureS() override;
  void CreateTextureR(TextureHandle handle, SamplerType target,
                      uint8_t levels, TextureFormat format,
                      uint8_t samples, uint32_t width,
                      uint32_t height, uint32_t depth, TextureUsage usage) override;
  void Update2DImage(TextureHandle handle, uint32_t level, uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height, PixelBufferDescriptor &&data) override;

  // 设置当前交换链
  void MakeCurrent(SwapChainHandle draw, SwapChainHandle read) override;

  // 提交命令，并刷新交换链
  void Commit(SwapChainHandle handle) override;

  void Draw(PipelineState state, RenderPrimitiveHandle handle) override;

  void Blit(TargetBufferFlags buffers, RenderTargetHandle dst, Viewport dst_rect, RenderTargetHandle src, Viewport src_rect, SamplerMagFilter filter) override;
 private:
  void GC();
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
  std::unique_ptr<VulkanBlitter> blitter_;

  VulkanRenderTarget* current_render_target_ {nullptr};
  VulkanSamplerGroup* sampler_bindings_[VulkanPipelineCache::SAMPLER_BINDING_COUNT] = {};


 private:

};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDRIVER_H_
