//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanRenderProcessor.h"

#include "Backend/Vulkan/VulkanInstance.h"
#include "Backend/Vulkan/VulkanDevice.h"
#include "Backend/Vulkan/VulkanMemoryAllocator.h"
#include "Backend/Vulkan/VulkanTexture.h"
#include "Backend/Vulkan/VulkanCommandPool.h"
#include "Framework/Backend/include/ITexture.h"
namespace our_graph {
void VulkanRenderProcessor::Init() {
  render_instance_ = std::make_shared<VulkanInstance>();
  render_device_ = std::make_shared<VulkanDevice>();
  render_instance_->CreateInstance();
  render_device_->CreateDevice(render_instance_);
  // 初始化显存管理
  MemoryAllocator::Get<VulkanMemoryAllocator>()->Init(render_device_);

  VkDevice device = dynamic_cast<VulkanDevice*>(render_device_.get())->GetDevice();
  int queue_idx = dynamic_cast<VulkanDevice*>(render_device_.get())->GetQueueFamilyIdx();
  command_buffer_ = std::make_shared<VulkanCommandPool>(device, queue_idx);
  command_buffer_->Create();
 }

void VulkanRenderProcessor::Destroy() {
  // 按照初始化的倒序销毁
  MemoryAllocator::Get<VulkanMemoryAllocator>()->Clear();
  command_buffer_->Destroy();
  render_device_->DestroyDevice();
  render_instance_->DestroyInstance();

}

void VulkanRenderProcessor::End() {
  texture_->Destroy();
}

void VulkanRenderProcessor::Start() {
  MemoryAllocator::Get<VulkanMemoryAllocator>()->AllocateGPUMemory("test", 104857600);
  texture_ = std::make_shared<VulkanTexture>();
  texture_->Create(render_device_);
}


void VulkanRenderProcessor::AfterRender() {

}

void VulkanRenderProcessor::BeforeRender() {

}

void VulkanRenderProcessor::Render() {

}

}  // namespace our_graph