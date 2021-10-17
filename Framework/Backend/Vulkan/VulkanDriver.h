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
#include "../include_internal/HandleAllocator.h"

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
  void EndRenderPass() override;

  RenderPrimitiveHandle CreateRenderPrimitive() override;
  void DestroyRenderPrimitive(RenderPrimitiveHandle handle) override;
  void SetRenderPrimitiveBuffer(RenderPrimitiveHandle handle, VertexBufferHandle vertex, IndexBufferHandle index) override;
  void SetRenderPrimitiveRange(RenderPrimitiveHandle handle, PrimitiveType type, uint32_t offset, uint32_t min_idx, uint32_t max_idx, uint32_t cnt) override;

  VertexBufferHandle CreateVertexBuffer(uint8_t buffer_cnt, uint8_t attribute_cnt, uint32_t vertex_cnt, AttributeArray attributes) override;
  void DestroyVertexBuffer(VertexBufferHandle handle) override;

  BufferObjectHandle CreateBufferObject(uint32_t bytes, BufferObjectBinding binding_type, BufferUsage usage) override;
  void DestroyBufferObject(BufferObjectHandle handle) override;

  void UpdateBufferObject(BufferObjectHandle handle, BufferDescriptor &&data, uint32_t byte_offset) override;
  void SetVertexBufferObject(VertexBufferHandle handle, uint32_t index, BufferObjectHandle buffer_handle) override;

  IndexBufferHandle CreateIndexBuffer(ElementType element_type, uint32_t index_cnt, BufferUsage usage) override;
  void DestroyIndexBuffer(IndexBufferHandle handle) override;
  void UpdateIndexBuffer(IndexBufferHandle handle, BufferDescriptor &&data, uint32_t byte_offset) override;

  void BindUniformBuffer(uint32_t idx, BufferObjectHandle handle) override;
  void BindUniformBufferRange(uint32_t idx, BufferObjectHandle handle, uint32_t offset, uint32_t size) override;

  ShaderHandle CreateShader(Program &&shaders) override;
  void DestroyShader(ShaderHandle handle) override;

  // 设置当前交换链
  void MakeCurrent(SwapChainHandle draw, SwapChainHandle read) override;

  // 提交命令，并刷新交换链
  void Commit(SwapChainHandle handle) override;

  void Draw(PipelineState state, RenderPrimitiveHandle handle) override;
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

  VulkanRenderTarget* current_render_target_ {nullptr};
  VulkanSamplerGroup* sampler_bindings_[VulkanPipelineCache::SAMPLER_BINDING_COUNT] = {};

  template<typename D, typename ... ARGS>
  Handle<D> InitHandle(ARGS&& ... args) noexcept {
    return HandleAllocator::Get().AllocateAndConstruct<D>(std::forward<ARGS>(args) ...);
  }

  template<typename D>
  Handle<D> AllocHandle() noexcept {
    return HandleAllocator::Get().Allocate<D>();
  }

  template<typename D, typename B, typename ... ARGS>
  typename std::enable_if<std::is_base_of<B, D>::value, D>::type*
  Construct(Handle<B> const& handle, ARGS&& ... args) noexcept {
    return HandleAllocator::Get().Construct<D, B>(handle, std::forward<ARGS>(args) ...);
  }

  template<typename B, typename D,
      typename = typename std::enable_if<std::is_base_of<B, D>::value, D>::type>
  void Destruct(Handle<B> handle, D const* p) noexcept {
    return HandleAllocator::Get().Deallocate(handle, p);
  }

  template<typename Dp, typename B>
  typename std::enable_if_t<
      std::is_pointer_v<Dp> &&
          std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
  HandleCast(Handle<B>& handle) noexcept {
    return HandleAllocator::Get().HandleCast<Dp, B>(handle);
  }

  template<typename Dp, typename B>
  inline typename std::enable_if_t<
      std::is_pointer_v<Dp> &&
          std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
  HandleCast(Handle<B> const& handle) noexcept {
    return HandleAllocator::Get(). HandleCast<Dp, B>(handle);
  }

  template<typename D, typename B>
  void Destruct(Handle<B> handle) noexcept {
    Destruct(handle, HandleCast<D const*>(handle));
  }

};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDRIVER_H_
