//
// Created by Glodxy on 2021/10/7.
//
#include "VulkanDriver.h"
#include "VulkanContext.h"
#include "VulkanTexture.h"
#include "VulkanSwapChain.h"
#include "VulkanDef.h"
#include "../include_internal/HandleAllocator.h"

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
  pipeline_cache_->SetDummyTexture(VulkanContext::Get().empty_texture->GetPrimaryImageView());

  // 设置深度格式
  std::vector<VkFormat> formats = {
      VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_X8_D24_UNORM_PACK32
  };
  VulkanContext::Get().final_depth_format_ = VulkanUtils::FindSupportedFormat(
      formats, VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

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

void VulkanDriver::Clear() {
  delete VulkanContext::Get().empty_texture;
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

void VulkanDriver::CreateEmptyTexture(VulkanStagePool &stage_pool) {
  VulkanContext::Get().empty_texture = new VulkanTexture(SamplerType::SAMPLER_2D, 1,
                                                         TextureFormat::RGBA8, 1, 1, 1, 1,
                                                         TextureUsage::DEFAULT | TextureUsage::COLOR_ATTACHMENT |
                                                         TextureUsage::SUBPASS_INPUT,
                                                         stage_pool);
  uint32_t black = 0;
  PixelBufferDescriptor desc(&black, 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
  VulkanContext::Get().empty_texture->Update2DImage(desc, 1, 1, 0);
}

}  // namespace our_graph