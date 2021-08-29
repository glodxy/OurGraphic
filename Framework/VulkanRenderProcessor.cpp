//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanRenderProcessor.h"

#include "Backend/Vulkan/VulkanInstance.h"
#include "Backend/Vulkan/VulkanDevice.h"
namespace our_graph {
void VulkanRenderProcessor::Init() {
  render_instance_ = std::make_shared<VulkanInstance>();
  render_device_ = std::make_shared<VulkanDevice>();
  render_instance_->CreateInstance();
  render_device_->CreateDevice(render_instance_);
}

void VulkanRenderProcessor::Destroy() {
  render_device_->DestroyDevice();
  render_instance_->DestroyInstance();
}

void VulkanRenderProcessor::End() {

}

void VulkanRenderProcessor::Start() {

}


void VulkanRenderProcessor::AfterRender() {

}

void VulkanRenderProcessor::BeforeRender() {

}

void VulkanRenderProcessor::Render() {

}

}  // namespace our_graph