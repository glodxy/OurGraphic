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

namespace our_graph {
void VulkanDriver::Init(std::unique_ptr<IPlatform> platform) {
  std::swap(platform_, platform);
  instance_ = std::make_unique<VulkanInstance>(platform_->GetInstanceExtLayers());
  instance_->CreateInstance();
  device_ = std::make_unique<VulkanDevice>();
  device_->CreateDevice(instance_.get());
  stage_pool_ = std::make_unique<VulkanStagePool>();

  CreateEmptyTexture(*stage_pool_);

  fbo_cache_ = std::make_unique<VulkanFBOCache>();
  pipeline_cache_ = std::make_unique<VulkanPipelineCache>();

  VulkanContext::Get().commands_->SetObserver(pipeline_cache_.get());
  pipeline_cache_->SetDevice(device_->GetDevice(), VulkanContext::Get().allocator_);
  pipeline_cache_->SetDummyTexture(VulkanContext::Get().empty_texture_->GetPrimaryImageView());

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

SwapChainHandle VulkanDriver::CreateSwapChain(void *native_window, uint64_t flags) {
  VkDevice device = *VulkanContext::Get().device_;
  VkInstance instance = VulkanContext::Get().instance_;
  SwapChainHandle handle =
      HandleAllocator::Get().AllocateAndConstruct<VulkanSwapChain>(device, instance, platform_.get(), native_window);
  return handle;
}

void VulkanDriver::DestroySwapChain(SwapChainHandle handle) {
  const VulkanSwapChain* p = HandleAllocator::Get().HandleCast<const VulkanSwapChain*>(handle);
  HandleAllocator::Get().Deallocate(handle, p);
}

RenderTargetHandle VulkanDriver::CreateDefaultRenderTarget() {
  RenderTargetHandle handle =
      HandleAllocator::Get().AllocateAndConstruct<VulkanRenderTarget>();
  const VulkanRenderTarget* render_target = HandleAllocator::Get().HandleCast<const VulkanRenderTarget*>(handle);
  disposer_->CreateDisposable(render_target, [this, render_target, handle](){
    HandleAllocator::Get().Deallocate(handle, render_target);
  });
  return handle;
}

void VulkanDriver::DestroyRenderTarget(RenderTargetHandle handle) {
  const VulkanRenderTarget* p = HandleAllocator::Get().HandleCast<const VulkanRenderTarget*>(handle);
  HandleAllocator::Get().Deallocate(handle, p);
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
    // todo: 清理垃圾
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
    //todo:清理垃圾
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
      .width = (float) params.viewport.width,
      .height = (float) params.viewport.height,
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

  current_render_target_ = VK_NULL_HANDLE;\
  // 不止一个subpass时, 重置
  if (VulkanContext::Get().current_render_pass_.currentSubpass > 0) {
    for (int i = 0; i < VulkanPipelineCache::TARGET_BINDING_COUNT; ++i) {
      pipeline_cache_->BindInputAttachment(i, {});
    }
    VulkanContext::Get().current_render_pass_.currentSubpass = 0;
  }
  VulkanContext::Get().current_render_pass_.renderPass = VK_NULL_HANDLE;
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

    const VulkanBuffer* buffer = primitive.vertex_buffer_->buffers_[attribute.buffer];
    // 不存在buffer时，跳过这次DrawCall
    if (!buffer){
      return;
    }

    buffers[i] = buffer->GetGPUBuffer();
    offsets[i] = attribute.offset;
    vertex_array.attribution[i] = {
        .location = (uint32_t)i,
        .binding = (uint32_t)i,
        .format = vk_format,
    };
    vertex_array.buffers[i] = {
        .binding = (uint32_t)i,
        .stride = attribute.stride,
    };
  }

  // 设置流水线状态
  pipeline_cache_->BindProgramBundle(program->bundle_);
  pipeline_cache_->BindRasterState(VulkanContext::Get().raster_state_);
  pipeline_cache_->BindPrimitiveTopology(primitive.primitive_topology_);
  pipeline_cache_->BindVertexArray(vertex_array);

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

  // 绑定Vertex与Index buffer
  vkCmdBindVertexBuffers(cmd_buffer, 0, buffer_cnt, buffers, offsets);
  vkCmdBindIndexBuffer(cmd_buffer, primitive.index_buffer_->buffer_->GetGPUBuffer(),
                       0, primitive.index_buffer_->index_type_);

  const uint32_t index_cnt = primitive.count_;
  const uint32_t instance_cnt = 1;
  const uint32_t first_index = primitive.offset_ / primitive.index_buffer_->element_size_;
  const int32_t vertex_offset = 0;
  const uint32_t first_instance_id = 1;
  vkCmdDrawIndexed(cmd_buffer, index_cnt, instance_cnt, first_index, vertex_offset, first_instance_id);
}

void VulkanDriver::Clear() {
  disposer_->Reset();
  delete VulkanContext::Get().empty_texture_;
  pipeline_cache_->DestroyAllCache();
  fbo_cache_->Reset();
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
      size_t binding_point = sampler.binding;
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