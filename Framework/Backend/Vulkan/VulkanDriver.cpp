//
// Created by Glodxy on 2021/10/7.
//
#include "VulkanDriver.h"
#include "VulkanContext.h"
#include "VulkanTexture.h"
#include "VulkanSwapChain.h"
#include "VulkanDef.h"
#include "../include_internal/HandleAllocator.h"
#include "VulkanHandles.h"
#include "../include/PipelineState.h"
#include "VulkanBuffer.h"
#include "../include_internal/Dispatcher.h"
#include "Backend/DataReshaper.h"

namespace our_graph {
VulkanDriver::VulkanDriver() noexcept : DriverApi() {
  dispatcher_ = new Dispatcher<VulkanDriver>();
}

VulkanDriver::~VulkanDriver() {
  delete dispatcher_;
}

void VulkanDriver::Init(std::unique_ptr<IPlatform> platform) {
  std::swap(platform_, platform);
  instance_ = std::make_unique<VulkanInstance>(platform_->GetInstanceExtLayers());
  instance_->CreateInstance();
  device_ = std::make_unique<VulkanDevice>();
  device_->CreateDevice(instance_.get());
  stage_pool_ = std::make_unique<VulkanStagePool>();

  fbo_cache_ = std::make_unique<VulkanFBOCache>();
  pipeline_cache_ = std::make_unique<VulkanPipelineCache>();
  sampler_cache_ = std::make_unique<VulkanSamplerCache>();

  VulkanContext::Get().commands_->SetObserver(pipeline_cache_.get());
  pipeline_cache_->SetDevice(device_->GetDevice(), VulkanContext::Get().allocator_);
  CreateEmptyTexture(*stage_pool_);

  pipeline_cache_->SetDummyTexture(VulkanContext::Get().empty_texture_->GetPrimaryImageView());

  blitter_ = std::make_unique<VulkanBlitter>(*stage_pool_, *pipeline_cache_, *fbo_cache_, *sampler_cache_);
  disposer_ = std::make_unique<VulkanDisposer>();
  // 设置深度格式
  std::vector<VkFormat> formats = {
      VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_X8_D24_UNORM_PACK32
  };
  VulkanContext::Get().final_depth_format_ = VulkanUtils::FindSupportedFormat(
      formats, VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  VulkanContext::Get().raster_state_ = pipeline_cache_->GetDefaultRasterSate();
}

SwapChainHandle VulkanDriver::CreateSwapChainS() {
  return AllocHandle<VulkanSwapChain>();
}

void VulkanDriver::CreateSwapChainR(
    SwapChainHandle handle,
    void *native_window, uint64_t flags) {
  VkDevice device = *VulkanContext::Get().device_;
  VkInstance instance = VulkanContext::Get().instance_;
  Construct<VulkanSwapChain>(handle, device, instance, platform_.get(), native_window);
}

RenderTargetHandle VulkanDriver::CreateRenderTargetS() {
  return AllocHandle<VulkanRenderTarget>();
}

void VulkanDriver::CreateRenderTargetR(RenderTargetHandle handle,
                                       TargetBufferFlags target_flags,
                                       uint32_t width,
                                       uint32_t height,
                                       uint8_t samples,
                                       MRT color,
                                       TargetBufferInfo depth,
                                       TargetBufferInfo stencil) {
  VulkanAttachment color_targets[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  // 设置每个rendertarget纹理
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++ i) {
    if (color[i].handle_) {
      color_targets[i].texture = HandleCast<VulkanTexture*>(color[i].handle_);
    }
    color_targets[i].level = color[i].level_;
    color_targets[i].layer = color[i].layer_;
  }

  VulkanAttachment depth_stencil[2] = {};
  TextureHandle depth_handle = depth.handle_;
  depth_stencil[0].texture = depth_handle ? HandleCast<VulkanTexture*>(depth_handle) : nullptr;
  depth_stencil[0].level = depth.level_;
  depth_stencil[0].layer = depth.layer_;

  depth_handle = stencil.handle_;
  depth_stencil[1].texture = depth_handle ? HandleCast<VulkanTexture*>(depth_handle) : nullptr;
  depth_stencil[1].level = stencil.level_;
  depth_stencil[1].layer = stencil.layer_;

  auto render_target = Construct<VulkanRenderTarget>(handle, width, height, samples, color_targets, depth_stencil, *stage_pool_.get());
  disposer_->CreateDisposable(render_target, [this, handle]() {
    Destruct<VulkanRenderTarget>(handle);
  });
}

void VulkanDriver::DestroySwapChain(SwapChainHandle handle) {
  const VulkanSwapChain* p = HandleAllocator::Get().HandleCast<const VulkanSwapChain*>(handle);
  if (VulkanContext::Get().current_surface_ == p) {
    VulkanContext::Get().current_surface_ = nullptr;
  }

  HandleAllocator::Get().Deallocate(handle, p);
}

ShaderHandle VulkanDriver::CreateShaderS() {
  return AllocHandle<VulkanShader>();
}

void VulkanDriver::CreateShaderR(
    ShaderHandle handle,
    Program &&shaders) {
  auto shader = Construct<VulkanShader>(handle, shaders);
  disposer_->CreateDisposable(shader, [this, shader, handle](){
    HandleAllocator::Get().Deallocate(handle, shader);
  });
}

void VulkanDriver::DestroyShader(ShaderHandle handle) {
  if (handle) {
    disposer_->RemoveReference(HandleAllocator::Get().HandleCast<VulkanShader*>(handle));
  }
}

RenderTargetHandle VulkanDriver::CreateDefaultRenderTargetS() {
  return AllocHandle<VulkanRenderTarget>();
}
void VulkanDriver::CreateDefaultRenderTargetR(
    RenderTargetHandle handle) {
  auto render_target = Construct<VulkanRenderTarget>(handle);
  disposer_->CreateDisposable(render_target, [this, render_target, handle](){
    HandleAllocator::Get().Deallocate(handle, render_target);
  });
}

void VulkanDriver::DestroyRenderTarget(RenderTargetHandle handle) {
  const VulkanRenderTarget* p = HandleAllocator::Get().HandleCast<const VulkanRenderTarget*>(handle);
  HandleAllocator::Get().Deallocate(handle, p);
}

RenderPrimitiveHandle VulkanDriver::CreateRenderPrimitiveS() {
  return AllocHandle<VulkanRenderPrimitive>();
}

void VulkanDriver::CreateRenderPrimitiveR(RenderPrimitiveHandle handle) {
  Construct<VulkanRenderPrimitive>(handle);
}

void VulkanDriver::DestroyRenderPrimitive(RenderPrimitiveHandle handle) {
  if (handle) {
    Destruct<VulkanRenderPrimitive>(handle);
  }
}

//////////////////////////Buffer////////////////

VertexBufferHandle VulkanDriver::CreateVertexBufferS() {
  return AllocHandle<VulkanVertexBuffer>();
}

void VulkanDriver::CreateVertexBufferR(VertexBufferHandle handle,
                                       uint8_t buffer_cnt,
                                       uint8_t attribute_cnt,
                                       uint32_t vertex_cnt,
                                       AttributeArray attributes) {
  auto vertex_buffer = Construct<VulkanVertexBuffer>(handle,
                                                     *stage_pool_.get(),
                                                     buffer_cnt,
                                                     attribute_cnt,
                                                     vertex_cnt,
                                                     attributes);
  disposer_->CreateDisposable(vertex_buffer, [this, handle]() {
    Destruct<VulkanVertexBuffer>(handle);
  });
}

void VulkanDriver::DestroyVertexBuffer(VertexBufferHandle handle) {
  if (handle) {
    VulkanVertexBuffer* buffer = HandleCast<VulkanVertexBuffer*>(handle);
    disposer_->RemoveReference(buffer);
  }
}

IndexBufferHandle VulkanDriver::CreateIndexBufferS() {
  return AllocHandle<VulkanIndexBuffer>();
}

void VulkanDriver::CreateIndexBufferR(
    IndexBufferHandle handle,
    ElementType element_type, uint32_t index_cnt, BufferUsage usage) {
  uint8_t element_size = (uint8_t) GetElementTypeSize(element_type);
  auto buffer = Construct<VulkanIndexBuffer>(handle, *stage_pool_.get(), element_size, index_cnt);

  disposer_->CreateDisposable(buffer, [this, handle]() {
    Destruct<VulkanIndexBuffer>(handle);
  });
}

void VulkanDriver::DestroyIndexBuffer(IndexBufferHandle handle) {
  if (handle) {
    auto index_buffer = HandleCast<VulkanIndexBuffer*>(handle);
    disposer_->RemoveReference(index_buffer);
  }
}

void VulkanDriver::UpdateIndexBuffer(
    IndexBufferHandle handle, BufferDescriptor &&data, uint32_t byte_offset) {
  auto ib = HandleCast<VulkanIndexBuffer*>(handle);
  ib->buffer_->LoadFromCPU(*stage_pool_.get(), data.buffer_, byte_offset, data.size_);
  disposer_->Acquire(ib);
  PurgeBuffer(std::move(data));
}

TextureHandle VulkanDriver::CreateTextureS() {
  return AllocHandle<VulkanTexture>();
}

void VulkanDriver::CreateTextureR(TextureHandle handle,
                                  SamplerType target,
                                  uint8_t levels,
                                  TextureFormat format,
                                  uint8_t samples,
                                  uint32_t width,
                                  uint32_t height,
                                  uint32_t depth,
                                  TextureUsage usage) {
  auto vk_texture =
      Construct<VulkanTexture>(handle, target, levels, format, samples,
                               width, height, depth, usage, *stage_pool_.get());

  disposer_->CreateDisposable(vk_texture, [this, handle] () {
    Destruct<VulkanTexture>(handle);
  });
}

void VulkanDriver::Update2DImage(TextureHandle handle,
                                 uint32_t level,
                                 uint32_t x_offset,
                                 uint32_t y_offset,
                                 uint32_t width,
                                 uint32_t height,
                                 PixelBufferDescriptor &&data) {
  HandleCast<VulkanTexture*>(handle)->Update2DImage(data, width, height, level);
  PurgeBuffer(std::move(data));
}

void VulkanDriver::UpdateCubeImage(TextureHandle handle,
                                   uint32_t level,
                                   PixelBufferDescriptor &&data,
                                   FaceOffsets face_offsets) {
  HandleCast<VulkanTexture*>(handle)->UpdateCubeImage(data, face_offsets, level);
  PurgeBuffer(std::move(data));
}


BufferObjectHandle VulkanDriver::CreateBufferObjectS() {
  return AllocHandle<VulkanBufferObject>();
}

void VulkanDriver::CreateBufferObjectR(
    BufferObjectHandle handle,
    uint32_t bytes,
    BufferObjectBinding binding_type,
    BufferUsage usage) {
  auto buffer = Construct<VulkanBufferObject>(handle, *stage_pool_.get(),
                                              bytes, binding_type, usage);
  disposer_->CreateDisposable(buffer, [this, handle]() {
    Destruct<VulkanBufferObject>(handle);
  });
}

void VulkanDriver::DestroyBufferObject(BufferObjectHandle handle) {
  if (handle) {
    auto buffer_obj = HandleCast<VulkanBufferObject*>(handle);
    if (buffer_obj->bind_type_ == BufferObjectBinding::UNIFORM) {
      // uniform buffer时，需要先从pipeline移除
      pipeline_cache_->UnBindUniformBuffer(buffer_obj->buffer_->GetGPUBuffer());
      disposer_->Acquire(buffer_obj);
    }
    disposer_->RemoveReference(buffer_obj);
  }
}

void VulkanDriver::UpdateBufferObject(
    BufferObjectHandle handle,
    BufferDescriptor &&data,
    uint32_t byte_offset) {
  if(!handle) {return;}
  auto buffer_obj = HandleCast<VulkanBufferObject*>(handle);
  buffer_obj->buffer_->LoadFromCPU(*stage_pool_.get(), data.buffer_, byte_offset, data.size_);
  disposer_->Acquire(buffer_obj);
  PurgeBuffer(std::move(data));
}

void VulkanDriver::SetVertexBufferObject(
    VertexBufferHandle handle,
    uint32_t index,
    BufferObjectHandle buffer_handle) {
  auto& vb = *HandleCast<VulkanVertexBuffer*>(handle);
  auto& bo = *HandleCast<VulkanBufferObject*>(buffer_handle);
  if (bo.bind_type_ != BufferObjectBinding::VERTEX) {
    LOG_ERROR("VulkanDriver", "SetVertexBufferObject Error!"
                              "Buffer Object Bind Type:{}", bo.bind_type_);
    return;
  }
  vb.buffers_[index] = bo.buffer_;
}

void VulkanDriver::BindUniformBuffer(
    uint32_t idx, BufferObjectHandle handle) {
  auto* bo = HandleCast<VulkanBufferObject*>(handle);
  const VkDeviceSize offset = 0;
  const VkDeviceSize size = VK_WHOLE_SIZE;
  pipeline_cache_->BindUniformBuffer(idx, bo->buffer_->GetGPUBuffer(), offset, size);
}

void VulkanDriver::BindUniformBufferRange(
    uint32_t idx, BufferObjectHandle handle,
    uint32_t offset, uint32_t size) {
  auto* bo = HandleCast<VulkanBufferObject*>(handle);
  pipeline_cache_->BindUniformBuffer(idx, bo->buffer_->GetGPUBuffer(), offset, size);
}

SamplerGroupHandle VulkanDriver::CreateSamplerGroupS() {
  return AllocHandle<VulkanSamplerGroup>();
}

void VulkanDriver::CreateSamplerGroupR(SamplerGroupHandle handle, uint32_t size) {
  Construct<VulkanSamplerGroup>(handle, size);
}

void VulkanDriver::BindSamplers(uint32_t idx, SamplerGroupHandle handle) {
  auto sg = HandleCast<VulkanSamplerGroup*>(handle);
  sampler_bindings_[idx] = sg;
}

void VulkanDriver::UpdateSamplerGroup(SamplerGroupHandle handle, SamplerGroup &&sampler_group) {
  auto* sb = HandleCast<VulkanSamplerGroup*>(handle);
  *(sb->sampler_group_) = sampler_group;
}

void VulkanDriver::DestroySamplerGroup(SamplerGroupHandle handle) {
  if (handle) {
    // Unlike most of the other "Hw" handles, the sampler buffer is an abstract concept and does
    // not map to any Vulkan objects. To handle destruction, the only thing we need to do is
    // ensure that the next draw call doesn't try to access a zombie sampler buffer. Therefore,
    // simply replace all weak references with null.
    auto* vk_sg = HandleCast<VulkanSamplerGroup*>(handle);
    for (auto& binding : sampler_bindings_) {
      if (binding == vk_sg) {
        binding = nullptr;
      }
    }
    Destruct<VulkanSamplerGroup>(handle);
  }
}
//////////////////Buffer////////////////
//////////////////////////////////////


void VulkanDriver::SetRenderPrimitiveBuffer(RenderPrimitiveHandle handle,
                                            VertexBufferHandle vertex,
                                            IndexBufferHandle index) {
  auto primitive = HandleCast<VulkanRenderPrimitive*>(handle);
  primitive->SetBuffers(HandleCast<VulkanVertexBuffer*>(vertex),
                        HandleCast<VulkanIndexBuffer*>(index));
}

void VulkanDriver::SetRenderPrimitiveRange(RenderPrimitiveHandle handle,
                                           PrimitiveType type,
                                           uint32_t offset,
                                           uint32_t min_idx,
                                           uint32_t max_idx,
                                           uint32_t cnt) {
  auto& primitive = *HandleCast<VulkanRenderPrimitive*>(handle);
  primitive.SetPrimitiveType(type);
  primitive.offset_ = primitive.index_buffer_ ? offset * primitive.index_buffer_->element_size_ : 0;
  primitive.count_ = cnt;
  primitive.min_index_ = min_idx;
  primitive.max_index_ = max_idx > min_idx? max_idx : primitive.max_vertex_cnt_ - 1;
}

void VulkanDriver::MakeCurrent(SwapChainHandle draw, SwapChainHandle read) {
  if (draw != read) {
    LOG_ERROR("VulkanSwapChain", "Not Supported Switch To Self!");
    return;
  }
  VulkanSwapChain& surf = *(HandleAllocator::Get().HandleCast<VulkanSwapChain*>(draw));
  VulkanContext::Get().current_surface_ = &surf;

  if (surf.IsAcquired()) {
    return;
  }

  if (surf.HasResized()) {
    RefreshSwapChain();
  }
  // 请求交换链的下一张image
  surf.Acquire();
}

void VulkanDriver::Commit(SwapChainHandle handle) {
  VulkanSwapChain& surface = *(HandleAllocator::Get().HandleCast<VulkanSwapChain*>(handle));

  // 设置交换链的属性，让其为展示在屏幕上做准备
  surface.MakePresentable();

  if (VulkanContext::Get().commands_->Commit()) {
    GC();
  }

  surface.SetFirstRenderPass(true);

  surface.SetAcquired(false);

  VkSemaphore render_finished = VulkanContext::Get().commands_->AcquireFinishedSignal();
  VkPresentInfoKHR present_info {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &render_finished,
    .swapchainCount = 1,
    .pSwapchains = surface.GetSwapChainPtr(),
    .pImageIndices = surface.GetCurrentIdxPtr(),
  };
  VkResult res = vkQueuePresentKHR(surface.GetPresentQueue(), &present_info);

  if (res == VK_SUBOPTIMAL_KHR && !surface.UsedSubOptimal()) {
    LOG_WARN("VulkanSwapChain", "Use SubOptimal SwapChain");
    surface.SetSubOptimal(true);
  }

  assert(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR ||
          res == VK_ERROR_OUT_OF_DATE_KHR);
}

void VulkanDriver::Tick() {
  VulkanContext::Get().commands_->UpdateFence();
}

void VulkanDriver::EndFrame(uint32_t frame_id) {
  if (VulkanContext::Get().commands_->Commit()) {
    GC();
  }
}

void VulkanDriver::BeginRenderPass(RenderTargetHandle handle, const RenderPassParams &params) {
  current_render_target_ = HandleAllocator::Get().HandleCast<VulkanRenderTarget*>(handle);
  VulkanRenderTarget *const rt = current_render_target_;
  VulkanSwapChain* const sc = VulkanContext::Get().current_surface_;

  const VkExtent2D extent = rt->GetExtent(sc);
  assert(extent.width > 0 && extent.height > 0);

  TargetBufferFlags discard_start = params.flags.discardStart;
  if (rt->IsSwapChain()) {
    assert(sc);
    VulkanSwapChain& surface = *sc;
    if (surface.IsFirstRenderPass()) {
      discard_start |= TargetBufferFlags::COLOR;
      surface.SetFirstRenderPass(false);
    }
  }

  const VkCommandBuffer  cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;
  VulkanAttachment depth = rt->GetDepth(sc);
  VulkanTexture* depth_feedback = nullptr;

  // 如果在renderpass 结束时会丢弃该depth，则需要改变其layout为只读
  if (depth.texture && any(params.flags.discardEnd & TargetBufferFlags::DEPTH) &&
      !any(params.flags.clear & TargetBufferFlags::DEPTH)) {
    depth_feedback = depth.texture;
    const VulkanLayoutTransition transition = {
        .image = depth.image,
        .oldLayout = depth.layout,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        .subresources = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = depth.texture->levels_,
            .layerCount = depth.texture->depth_,
        },
        .srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    };
    VulkanUtils::TransitionImageLayout(cmd_buffer, transition);
    depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  }

  VulkanFBOCache::RenderPassKey rp_key = {
      .depth_layout = depth.layout,
      .depth_format = depth.format,
      .clear = params.flags.clear,
      .discard_start = discard_start,
      .discard_end = params.flags.discardEnd,
      .samples = rt->GetSamples(),
      .subpass_mask = uint8_t(params.subpassMask)
  };
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    rp_key.color_layout[i] = rt->GetColor(sc, i).layout;
    rp_key.color_format[i] = rt->GetColor(sc, i).format;
    VulkanTexture* texture = rt->GetColor(sc, i).texture;
    // 如果render pass的采样数大于1， 但该纹理采样数为1，则需要将其标记为需要生成高采样纹理
    if (rp_key.samples > 1 && texture && texture->samples_ == 1) {
      rp_key.needs_resolve_mask |= (1 << i);
    }
  }

  VkRenderPass render_pass = fbo_cache_->GetRenderPass(rp_key);
  pipeline_cache_->BindRenderPass(render_pass, 0);

  VulkanFBOCache::FBOKey fb_key = {
      .render_pass = render_pass,
      .width = (uint16_t) extent.width,
      .height = (uint16_t) extent.height,
      .layers = 1,
      .samples = rp_key.samples
  };
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (rt->GetColor(sc, i).format == VK_FORMAT_UNDEFINED) {
      fb_key.color[i] = VK_NULL_HANDLE;
      fb_key.resolve[i] = VK_NULL_HANDLE;
    } else if (fb_key.samples == 1) {
      fb_key.color[i] = rt->GetColor(sc, i).view;
      fb_key.resolve[i] = VK_NULL_HANDLE;
      assert(fb_key.color[i]);
    } else {
      fb_key.color[i] = rt->GetMsaaColor(i).view;
      VulkanTexture* texture = rt->GetColor(sc, i).texture;
      if (texture && texture->samples_ == 1) {
        fb_key.resolve[i] = rt->GetColor(sc, i).view;
        assert(fb_key.resolve[i]);
      }
      assert(fb_key.color[i]);
    }
  }

  if (depth.format != VK_FORMAT_UNDEFINED) {
    fb_key.depth = (rp_key.samples == 1 ? depth.view : rt->GetMsaaDepth().view);
    assert(fb_key.depth);
  }

  VkFramebuffer vk_fb = fbo_cache_->GetFrameBuffer(fb_key);

  // 标识资源的使用
  disposer_->Acquire(rt);
  disposer_->Acquire(depth.texture);
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    disposer_->Acquire(rt->GetColor(sc, i).texture);
  }

  VkRenderPassBeginInfo render_pass_info {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = render_pass,
    .framebuffer = vk_fb,
    .renderArea = {.offset = {}, .extent = extent},
  };

  rt->TransformClientRectToPlatform(sc, &render_pass_info.renderArea);

  VkClearValue clear_values[MAX_SUPPORTED_RENDER_TARGET_COUNT + MAX_SUPPORTED_RENDER_TARGET_COUNT + 1] = {};

  // 设置color的clear
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (fb_key.color[i]) {
      VkClearValue& clear_value = clear_values[render_pass_info.clearValueCount++];
      clear_value.color.float32[0] = params.clearColor.r;
      clear_value.color.float32[1] = params.clearColor.g;
      clear_value.color.float32[2] = params.clearColor.b;
      clear_value.color.float32[3] = params.clearColor.a;
    }
  }
  // 设置resolve attachment的clear
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (rp_key.needs_resolve_mask & (1u << i)) {
      render_pass_info.clearValueCount ++;
    }
  }
  // 设置depth的clear
  if (fb_key.depth) {
    VkClearValue& clear_value = clear_values[render_pass_info.clearValueCount++];
    clear_value.depthStencil = {(float) params.clearDepth, 0};
  }

  render_pass_info.pClearValues = &clear_values[0];

  vkCmdBeginRenderPass(cmd_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport = VulkanContext::Get().viewport_ = {
      .x = (float) params.viewport.left,
      .y = (float) params.viewport.bottom,
      .width = (float) params.viewport.width == 0 ? (float)extent.width : params.viewport.width,
      .height = (float) params.viewport.height == 0 ? (float)extent.height : params.viewport.height,
      .minDepth = params.depthRange.near_,
      .maxDepth = params.depthRange.far_,
  };

  current_render_target_->TransformClientRectToPlatform(sc, &viewport);
  vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

  VulkanContext::Get().current_render_pass_ = {
      .renderPass = render_pass_info.renderPass,
      .subpassMask = params.subpassMask,
      .currentSubpass = 0,
      .depthFeedback = depth_feedback
  };
}

void VulkanDriver::EndRenderPass() {
  VkCommandBuffer cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;
  vkCmdEndRenderPass(cmd_buffer);

  // 设置render target的状态，使其可以被之后的pass 的shader读取
  VulkanTexture* depth_feedback = VulkanContext::Get().current_render_pass_.depthFeedback;
  if (depth_feedback) {
    const VulkanLayoutTransition transition = {
        .image = depth_feedback->GetVKImage(),
        .oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .subresources = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = depth_feedback->levels_,
            .layerCount = depth_feedback->depth_,
        },
        .srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    };
    VulkanUtils::TransitionImageLayout(cmd_buffer, transition);
  }

  if (!current_render_target_->IsSwapChain()) {
    // 当前rt未作为展示对象时，设置其为可读
    VkMemoryBarrier barrier {
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
    };
    VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (current_render_target_->HasDepth()) {
      barrier.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      src_stage_mask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    vkCmdPipelineBarrier(cmd_buffer, src_stage_mask,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT |
                         VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 1, &barrier, 0, nullptr, 0, nullptr);
  }

  current_render_target_ = VK_NULL_HANDLE;
  // 不止一个subpass时, 设置下一个subpass的输入
  if (VulkanContext::Get().current_render_pass_.currentSubpass > 0) {
    for (int i = 0; i < VulkanPipelineCache::TARGET_BINDING_COUNT; ++i) {
      pipeline_cache_->BindInputAttachment(i, {});
    }
    VulkanContext::Get().current_render_pass_.currentSubpass = 0;
  }
  VulkanContext::Get().current_render_pass_.renderPass = VK_NULL_HANDLE;
}

void VulkanDriver::NextSubPass() {
  VulkanSwapChain* const swap_chain = VulkanContext::Get().current_surface_;
  auto cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;

  vkCmdNextSubpass(cmd_buffer, VK_SUBPASS_CONTENTS_INLINE);

  pipeline_cache_->BindRenderPass(VulkanContext::Get().current_render_pass_.renderPass,
                                  ++ VulkanContext::Get().current_render_pass_.currentSubpass);

  for (uint32_t i = 0; i < VulkanPipelineCache::TARGET_BINDING_COUNT; ++i) {
    if ((1 << i) & VulkanContext::Get().current_render_pass_.subpassMask) {
      VulkanAttachment subpass_input = current_render_target_->GetColor(swap_chain, i);
      VkDescriptorImageInfo info = {
          .imageView = subpass_input.view,
          .imageLayout = subpass_input.layout,
      };
      pipeline_cache_->BindInputAttachment(i, info);
    }
  }
}


void VulkanDriver::Draw(PipelineState state, RenderPrimitiveHandle handle) {
  VkCommandBuffer cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;
  const VulkanRenderPrimitive& primitive = *HandleAllocator::Get().HandleCast<VulkanRenderPrimitive*>(handle);

  ShaderHandle shader_handle = state.shader_;
  RasterState raster_state = state.raster_state_;
  PolygonOffset depth_offset = state.polygon_offset_;
  Viewport view_port = state.scissor_;

  auto* program = HandleAllocator::Get().HandleCast<VulkanShader*>(shader_handle);
  disposer_->Acquire(program);
  disposer_->Acquire(primitive.index_buffer_);
  disposer_->Acquire(primitive.vertex_buffer_);

  const VulkanRenderTarget* rt = current_render_target_;
  SetupRasterState(rt, raster_state, depth_offset);

  VulkanPipelineCache::VertexArray vertex_array = {};
  VkBuffer buffers[MAX_VERTEX_ATTRIBUTE_COUNT] = {};
  VkDeviceSize offsets[MAX_VERTEX_ATTRIBUTE_COUNT] = {};

  if (primitive.vertex_buffer_) {
    const uint32_t buffer_cnt = primitive.vertex_buffer_->attributes_.size();
    // 构造vertex attributes
    for (int i = 0; i < buffer_cnt; ++i) {
      Attribute attribute = primitive.vertex_buffer_->attributes_[i];
      const bool is_integer = attribute.flags & Attribute::FLAG_INTEGER_TARGET;
      const bool is_normalized = attribute.flags & Attribute::FLAG_NORMALIZED;

      VkFormat vk_format = VulkanUtils::GetVkFormat(attribute.type, is_normalized, is_integer);

      // 如果该attribute未使用buffer，则赋予其空buffer
      if (attribute.buffer == Attribute::BUFFER_UNUSED) {
        vk_format = is_integer ? VK_FORMAT_R8G8B8A8_UINT : VK_FORMAT_R8G8B8A8_SNORM;
        attribute = primitive.vertex_buffer_->attributes_[0];
      }

      const VulkanBuffer *buffer = primitive.vertex_buffer_->buffers_[attribute.buffer];
      // 不存在buffer时，跳过这次DrawCall
      if (!buffer) {
        return;
      }

      buffers[i] = buffer->GetGPUBuffer();
      offsets[i] = attribute.offset;
      vertex_array.attribution[i] = {
          .location = (uint32_t) i,
          .binding = (uint32_t) i,
          .format = vk_format,
      };
      vertex_array.buffers[i] = {
          .binding = (uint32_t) i,
          .stride = attribute.stride,
      };
    }
    pipeline_cache_->BindVertexArray(vertex_array);
    // 绑定Vertex与Index buffer
    vkCmdBindVertexBuffers(cmd_buffer, 0, buffer_cnt, buffers, offsets);
  }
  // 设置流水线状态
  pipeline_cache_->BindProgramBundle(program->bundle_);
  pipeline_cache_->BindRasterState(VulkanContext::Get().raster_state_);
  pipeline_cache_->BindPrimitiveTopology(primitive.primitive_topology_);

  // 设置Sampler
  SetupSamplers(program);
  // 如果生成新的DescriptorSet失败，则跳过这次drawcall
  if (!pipeline_cache_->BindDescriptors(cmd_buffer)) {
    return;
  }



  // 设置裁剪范围
  SetupScissor(view_port, rt, cmd_buffer);

  // 创建新的pipeline
  pipeline_cache_->BindPipeline(cmd_buffer);

  if (primitive.index_buffer_) {
    vkCmdBindIndexBuffer(cmd_buffer, primitive.index_buffer_->buffer_->GetGPUBuffer(),
                         0, primitive.index_buffer_->index_type_);
    const uint32_t index_cnt = primitive.count_;
    const uint32_t instance_cnt = 1;
    const uint32_t first_index = primitive.offset_ / primitive.index_buffer_->element_size_;
    const int32_t vertex_offset = 0;
    const uint32_t first_instance_id = 1;

    vkCmdDrawIndexed(cmd_buffer, index_cnt, instance_cnt, first_index, vertex_offset, first_instance_id);
  } else {
    LOG_INFO("VulkanDriver", "Draw Call!");
    vkCmdDraw(cmd_buffer, 6, 1, 0, 0);
  }
}

void VulkanDriver::Blit(TargetBufferFlags buffers,
                        RenderTargetHandle dst,
                        Viewport dst_rect,
                        RenderTargetHandle src,
                        Viewport src_rect,
                        SamplerMagFilter filter) {
  if (VulkanContext::Get().current_render_pass_.renderPass != VK_NULL_HANDLE) {
    LOG_ERROR("VulkanDriver", "Blit Error! in render pass!");
    return;
  }
  // 希望只设置了COLOR0，如果有设置其他项直接报错
  if (any(buffers & (TargetBufferFlags::COLOR_ALL & ~TargetBufferFlags::COLOR0))) {
    LOG_ERROR("VulkanDriver", "Blit Error, Color Only Support Color0!");
    return;
  }

  VulkanRenderTarget* src_target = HandleCast<VulkanRenderTarget*>(src);
  VulkanRenderTarget* dst_target = HandleCast<VulkanRenderTarget*>(dst);

  VkFilter vk_filter = filter == SamplerMagFilter::NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
  const VkExtent2D src_extent = src_target->GetExtent(VulkanContext::Get().current_surface_);
  const int32_t src_left = std::min(src_rect.left, (int32_t) src_extent.width);
  const int32_t src_bottom = std::min(src_rect.bottom, (int32_t) src_extent.height);
  const int32_t src_right = std::min(src_rect.left + src_rect.width, src_extent.width);
  const int32_t src_top = std::min(src_rect.bottom + src_rect.height, src_extent.height);
  // 分别为左下与右上
  const VkOffset3D src_offsets[2] = { { src_left, src_bottom, 0 }, { src_right, src_top, 1 }};

  const VkExtent2D dst_extent = dst_target->GetExtent(VulkanContext::Get().current_surface_);
  const int32_t dst_left = std::min(dst_rect.left, (int32_t) dst_extent.width);
  const int32_t dst_bottom = std::min(dst_rect.bottom, (int32_t) dst_extent.height);
  const int32_t dst_right = std::min(dst_rect.left + dst_rect.width, dst_extent.width);
  const int32_t dst_top = std::min(dst_rect.bottom + dst_rect.height, dst_extent.height);
  const VkOffset3D dst_offsets[2] = { { dst_left, dst_bottom, 0 }, { dst_right, dst_top, 1 }};

  if (any(buffers & TargetBufferFlags::DEPTH) && src_target->HasDepth() && dst_target->HasDepth()) {
    blitter_->BlitDepth({dst_target, dst_offsets, src_target, src_offsets});
  }

  if (any(buffers & TargetBufferFlags::COLOR0)) {
    blitter_->BlitColor({ dst_target, dst_offsets, src_target, src_offsets, vk_filter, int(0) });
  }
}

void VulkanDriver::Clear() {
  disposer_->Reset();
  delete VulkanContext::Get().empty_texture_;

  pipeline_cache_->DestroyAllCache();
  fbo_cache_->Reset();
  sampler_cache_->Reset();
  stage_pool_->GC();
  stage_pool_->Reset();

  vmaDestroyPool(VulkanContext::Get().allocator_,
                 VulkanContext::Get().vma_pool_cpu_);
  vmaDestroyPool(VulkanContext::Get().allocator_,
                 VulkanContext::Get().vma_pool_gpu_);
  vmaDestroyAllocator(VulkanContext::Get().allocator_);

  device_->DestroyDevice();
  instance_->DestroyInstance();
}

void VulkanDriver::Flush() {
  VulkanContext::Get().commands_->Commit();
}

void VulkanDriver::Finish() {
  VulkanContext::Get().commands_->Commit();
}

void VulkanDriver::ReadPixels(RenderTargetHandle src,
                              size_t idx,
                              uint32_t x,
                              uint32_t y,
                              uint32_t width,
                              uint32_t height,
                              PixelBufferDescriptor &&buffer) {
  const VkDevice device = *VulkanContext::Get().device_;
  const VulkanRenderTarget* rt = HandleCast<VulkanRenderTarget*>(src);
  const VulkanTexture* src_tex = rt->GetColor(VulkanContext::Get().current_surface_, idx).texture;
  const VkFormat src_format = src_tex ? src_tex->GetVKFormat() : VulkanContext::Get().current_surface_->GetSurfaceFormat();
  const bool swizzle = src_format == VK_FORMAT_R8G8B8A8_UNORM;

  // 创建一个临时的image
  VkImageCreateInfo image_info {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = src_format,
      .extent = { width, height, 1 },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_LINEAR,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  VkImage stage_image;
  vkCreateImage(device, &image_info, nullptr, &stage_image);

  VkMemoryRequirements mem_reqs;
  VkDeviceMemory stage_memory;
  vkGetImageMemoryRequirements(device, stage_image, &mem_reqs);
  VkMemoryAllocateInfo alloc_info {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = mem_reqs.size,
    .memoryTypeIndex = VulkanUtils::SelectMemoryType(mem_reqs.memoryTypeBits,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
  };
  vkAllocateMemory(device, &alloc_info, nullptr, &stage_memory);
  vkBindImageMemory(device, stage_image, stage_memory, 0);

  VulkanContext::Get().commands_->Commit();
  VulkanContext::Get().commands_->Wait();

  // 切换layout
  const VkCommandBuffer cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;
  // 将stage image转换为传输状态
  VulkanUtils::TransitionImageLayout(cmd_buffer, {
      .image = stage_image,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .subresources = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      },
      .srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      .srcAccessMask = 0,
      .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
  });

  const VulkanAttachment src_attachment = rt->GetColor(VulkanContext::Get().current_surface_, idx);
  // 将image从rt拷贝到刚刚创建的image
  VkImageCopy copy_info = {
      .srcSubresource = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .mipLevel = src_attachment.level,
          .baseArrayLayer = src_attachment.layer,
          .layerCount = 1,
      },
      .srcOffset = {
          .x = (int32_t) x,
          .y = (int32_t) y,
      },
      .dstSubresource = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .layerCount = 1,
      },
      .extent = {
          .width = width,
          .height = height,
          .depth = 1,
      },
  };

  // 修改来源render target的状态
  const VkImageSubresourceRange src_range = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = src_attachment.level,
      .levelCount = 1,
      .baseArrayLayer = src_attachment.layer,
      .layerCount = 1,
  };
  VkImage src_image = rt->GetColor(VulkanContext::Get().current_surface_, idx).image;
  // 将render target的image修改为传输状态
  VulkanUtils::TransitionImageLayout(cmd_buffer, {
     .image = src_image,
     .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
     .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
     .subresources = src_range,
     .srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
     .srcAccessMask = 0,
     .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
     .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
  });

  // copy
  vkCmdCopyImage(cmd_buffer, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                 stage_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);

  // 需要将render target的状态重置
  if (src_tex || VulkanContext::Get().current_surface_->GetPresentQueue()) {
    const VkImageLayout present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VulkanUtils::TransitionImageLayout(cmd_buffer, {
        .image = src_image,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = src_tex ? VulkanUtils::GetTextureLayout(src_tex->usage_) : present,
        .subresources = src_range,
        .srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
    });
  } else {
    VulkanUtils::TransitionImageLayout(cmd_buffer, {
        .image = src_image,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .subresources = src_range,
        .srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
    });
  }


  // 将stage image的状态修改为general
  VulkanUtils::TransitionImageLayout(cmd_buffer, {
    .image = stage_image,
    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .newLayout = VK_IMAGE_LAYOUT_GENERAL,
    .subresources = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
          },
    .srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
    .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
  });
  VulkanContext::Get().commands_->Commit();
  VulkanContext::Get().commands_->Wait();

  VkImageSubresource sub_resource { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT };
  VkSubresourceLayout sub_resource_layout;
  vkGetImageSubresourceLayout(device, stage_image, &sub_resource, &sub_resource_layout);

  // 映射stage image的数据来进行拷贝
  const uint8_t* src_pixels;
  vkMapMemory(device, stage_memory, 0, VK_WHOLE_SIZE, 0, (void**) &src_pixels);
  src_pixels += sub_resource_layout.offset;
  //! 因为使用来view来反转y，所以image的y都是反的
  const bool flip_y = true;

  if (!DataReshaper::reshapeImage(&buffer, VulkanUtils::GetComponentType(src_format), src_pixels,
                                  sub_resource_layout.rowPitch, width, height, swizzle, flip_y)) {
    LOG_ERROR("ReadPixels", "ReadPixels to memory failed!");
  }

  vkUnmapMemory(device, stage_memory);

  disposer_->CreateDisposable((void*)stage_image, [=](){
    vkDestroyImage(device, stage_image, nullptr);
    vkFreeMemory(device, stage_memory, nullptr);
  });

  PurgeBuffer(std::move(buffer));
}

/////////////////////////////////////////
void VulkanDriver::GC() {
  stage_pool_->GC();
  fbo_cache_->GC();
  disposer_->GC();
  VulkanContext::Get().commands_->GC();
}

void VulkanDriver::RefreshSwapChain() {
  VulkanSwapChain& surface = *VulkanContext::Get().current_surface_;
  surface.Destroy();
  surface.Create();

  fbo_cache_->Reset();
}


void VulkanDriver::CreateEmptyTexture(VulkanStagePool &stage_pool) {
  VulkanContext::Get().empty_texture_ = new VulkanTexture(SamplerType::SAMPLER_2D, 1,
                                                          TextureFormat::RGBA8, 1, 1, 1, 1,
                                                          TextureUsage::DEFAULT | TextureUsage::COLOR_ATTACHMENT |
                                                         TextureUsage::SUBPASS_INPUT,
                                                          stage_pool);
  uint32_t black = 0;
  PixelBufferDescriptor desc(&black, 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
  VulkanContext::Get().empty_texture_->Update2DImage(desc, 1, 1, 0);
}

void VulkanDriver::SetupRasterState(const VulkanRenderTarget *rt,
                                    RasterState raster_state,
                                    PolygonOffset depth_offset) {
  VulkanContext::Get().raster_state_.depth_stencil = {
      .depth_test_enable = VK_TRUE,
      .depth_write_enable = (VkBool32) raster_state.depthWrite,
      .depth_compare_op = VulkanUtils::GetCompareOp(raster_state.depthFunc),
      .depth_bounds_test_enable = VK_FALSE,
      .stencil_test_enable = VK_FALSE,
  };

  VulkanContext::Get().raster_state_.multisampling = {
      .rasterization_samples = (VkSampleCountFlagBits) rt->GetSamples(),
      .alpha_to_coverage_enable = raster_state.alphaToCoverage,
  };

  VulkanContext::Get().raster_state_.blending = {
      .blendEnable = (VkBool32) raster_state.hasBlending(),
      .srcColorBlendFactor = VulkanUtils::GetBlendFactor(raster_state.blendFunctionSrcRGB),
      .dstColorBlendFactor = VulkanUtils::GetBlendFactor(raster_state.blendFunctionDstRGB),
      .colorBlendOp = (VkBlendOp)raster_state.blendEquationRGB,
      .srcAlphaBlendFactor = VulkanUtils::GetBlendFactor(raster_state.blendFunctionSrcAlpha),
      .dstAlphaBlendFactor = VulkanUtils::GetBlendFactor(raster_state.blendFunctionDstAlpha),
      .alphaBlendOp = (VkBlendOp) raster_state.blendEquationAlpha,
      .colorWriteMask = (VkColorComponentFlags)(raster_state.colorWrite? 0xf : 0x0),
  };

  auto& vk_raster = VulkanContext::Get().raster_state_.rasterization;
  vk_raster.cull_mode = VulkanUtils::GetCullMode(raster_state.culling);
  vk_raster.front_face = VulkanUtils::GetFrontFace(raster_state.inverseFrontFaces);
  vk_raster.depth_bias_enable = (depth_offset.constant || depth_offset.slope) ? VK_TRUE : VK_FALSE;
  vk_raster.depth_bias_constant_factor = depth_offset.constant;
  vk_raster.depth_bias_slope_factor = depth_offset.slope;

  VulkanContext::Get().raster_state_.color_target_count = rt->GetColorTargetCount(VulkanContext::Get().current_render_pass_);
}

void VulkanDriver::SetupSamplers(VulkanShader *program) {
  // 设置Sampler
  VkDescriptorImageInfo samplers[VulkanPipelineCache::SAMPLER_BINDING_COUNT] = {};
  for (int i = 0; i < Program::BINDING_COUNT; ++i) {
    //!< 此处的sampler group为槽，与后面获取的sb为一一对应的关系

    const auto& sampler_group = program->sampler_group_info_[i];
    if (sampler_group.empty()) {
      continue;
    }
    // 取出当前这组sampler group
    VulkanSamplerGroup* vk_sb = sampler_bindings_[i];
    if (!vk_sb) {
      continue;
    }
    SamplerGroup* sb = vk_sb->sampler_group_.get();
    assert(sb->GetSize() == sampler_group.size());
    size_t sampler_idx = 0;
    // 将该组sampler绑定至指定槽
    for (const auto& sampler : sampler_group) {
      // 获取应该绑定的位置
      size_t binding_point = sampler.binding;
      // 获取需要绑定的sampler
      const SamplerGroup::Sampler* bound_sampler = sb->GetSamplers() + sampler_idx;
      ++sampler_idx;

      const VulkanTexture* texture;
      // 如果当前sampler未绑定texture,则设置空纹理
      if (!bound_sampler->t) {
        if (!sampler.strict) {
          //如果不必绑定，直接跳过
          continue;
        }

        texture = VulkanContext::Get().empty_texture_;
      } else {
        texture = HandleAllocator::Get().HandleCast<const VulkanTexture*>(bound_sampler->t);
        disposer_->Acquire(texture);
      }

      // 将sampler写入DescriptorImageInfo数组
      const SamplerParams& sampler_params = bound_sampler->s;
      VkSampler vk_sampler = sampler_cache_->GetSampler(sampler_params);

      samplers[binding_point] = {
          .sampler = vk_sampler,
          .imageView = texture->GetPrimaryImageView(),
          .imageLayout = VulkanUtils::GetTextureLayout(texture->usage_)
      };

      // 如果该texture被用于当前render pass的depth 回写，则修改layout
      if (VulkanContext::Get().current_render_pass_.depthFeedback == texture) {
        samplers[binding_point].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
      }
    }
  }
  pipeline_cache_->BindSamplers(samplers);
}

void VulkanDriver::SetupScissor(Viewport view_port,
                                const VulkanRenderTarget *rt,
                                VkCommandBuffer cmd_buffer) {
  VkViewport context_viewport = VulkanContext::Get().viewport_;
  const int32_t x = std::max(view_port.left, (int32_t)context_viewport.x);
  const int32_t y = std::max(view_port.bottom, (int32_t)context_viewport.y);
  const int32_t right = std::min(view_port.left + (int32_t)view_port.width,
                                 (int32_t)(context_viewport.x + context_viewport.width));
  const int32_t top = std::min(view_port.bottom + (int32_t)view_port.height,
                               (int32_t)(context_viewport.y + context_viewport.height));
  VkRect2D scissor {
      .offset = {std::max(0, x), std::max(0, y)},
      .extent = {(uint32_t)right - x, (uint32_t)top - y}
  };

  rt->TransformClientRectToPlatform(VulkanContext::Get().current_surface_, &scissor);
  pipeline_cache_->BindScissor(cmd_buffer, scissor);
}

}  // namespace our_graph