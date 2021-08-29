//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanRenderProcessor.h"

#include "Backend/Vulkan/VulkanInstance.h"
#include "Backend/Vulkan/VulkanDevice.h"
#include "Backend/Vulkan/VulkanCommandBuffer.h"
#include "Backend/Vulkan/VulkanMemoryAllocator.h"
namespace our_graph {
void VulkanRenderProcessor::Init() {
  render_instance_ = std::make_shared<VulkanInstance>();
  render_device_ = std::make_shared<VulkanDevice>();
  command_buffer_ = std::make_shared<VulkanCommandBuffer>();
  render_instance_->CreateInstance();
  render_device_->CreateDevice(render_instance_);
  command_buffer_->Create();
  // 初始化显存管理
  IMemoryAllocator::Get<VulkanMemoryAllocator>()->Init(render_device_);
}

void VulkanRenderProcessor::Destroy() {
  // 按照初始化的倒序销毁
  IMemoryAllocator::Get<VulkanMemoryAllocator>()->Clear();
  command_buffer_->Destroy();
  render_device_->DestroyDevice();
  render_instance_->DestroyInstance();

}

void VulkanRenderProcessor::End() {

}

void VulkanRenderProcessor::Start() {
  IMemoryAllocator::Get<VulkanMemoryAllocator>()->AllocateGPUMemory("test", 104857600);
}


void VulkanRenderProcessor::AfterRender() {

}

void VulkanRenderProcessor::BeforeRender() {

}

void VulkanRenderProcessor::Render() {

}

}  // namespace our_graph