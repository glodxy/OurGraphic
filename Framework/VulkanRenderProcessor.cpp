//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanRenderProcessor.h"

#include "Backend/Vulkan/VulkanInstance.h"
#include "Backend/Vulkan/VulkanDevice.h"
#include "Backend/Vulkan/VulkanMemoryAllocator.h"
#include "Backend/Vulkan/VulkanTexture.h"
#include "Backend/Vulkan/VulkanCommandPool.h"
#if __APPLE__
#include "Backend/Vulkan/VulkanPlatformMacos.h"
#elif WIN32
#include "Backend/Vulkan/VulkanPlatformWindows.h"
#endif
#include "Framework/Backend/include/ITexture.h"
#include "Backend/Vulkan/VulkanSwapChain.h"
namespace our_graph {
void VulkanRenderProcessor::Init() {
  std::shared_ptr<IPlatform> platform = nullptr;
#if __APPLE__
  platform = std::make_shared<VulkanPlatformMacos>();
#elif WIN32
  platform = std::make_shared<VulkanPlatformWindows>();
#endif
  render_instance_ = std::make_shared<VulkanInstance>(platform->GetInstanceExtLayers());
  render_device_ = std::make_shared<VulkanDevice>();
  render_instance_->CreateInstance();
  render_device_->CreateDevice(render_instance_);
  // 初始化显存管理
  MemoryAllocator::Get<VulkanMemoryAllocator>()->Init(render_device_);

  VkDevice device = dynamic_cast<VulkanDevice*>(render_device_.get())->GetDevice();
  VkInstance instance = dynamic_cast<VulkanInstance*>(render_instance_.get())->GetInstance();
  int queue_idx = dynamic_cast<VulkanDevice*>(render_device_.get())->GetQueueFamilyIdx();
  command_buffer_ = std::make_shared<VulkanCommandPool>(device, queue_idx);
  command_buffer_->Create();

  swapchain_ = std::make_shared<VulkanSwapChain>(device, instance, platform);
 }

void VulkanRenderProcessor::Destroy() {
  // 按照初始化的倒序销毁
  MemoryAllocator::Get<VulkanMemoryAllocator>()->Clear();
  command_buffer_->Destroy();
  render_device_->DestroyDevice();
  render_instance_->DestroyInstance();

}

void VulkanRenderProcessor::End() {
}

void VulkanRenderProcessor::Start() {
  MemoryAllocator::Get<VulkanMemoryAllocator>()->AllocateGPUMemoryByIdx("test", 104857600);
}


void VulkanRenderProcessor::AfterRender() {

}

void VulkanRenderProcessor::BeforeRender() {

}

void VulkanRenderProcessor::Render() {

}

}  // namespace our_graph