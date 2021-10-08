//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanRenderProcessor.h"

#include "Backend/Vulkan/VulkanDriver.h"
#include "DriverContext.h"
#if __APPLE__
#include "Backend/Vulkan/VulkanPlatformMacos.h"
#elif WIN32
#include "Backend/Vulkan/VulkanPlatformWindows.h"
#endif
#include "Backend/Vulkan/VulkanSwapChain.h"
namespace our_graph {
void VulkanRenderProcessor::Init() {
  std::unique_ptr<IPlatform> platform;
#if __APPLE__
  platform = std::make_unique<VulkanPlatformMacos>();
#elif WIN32
  platform = std::make_unique<VulkanPlatformWindows>();
#endif
  driver_ = new VulkanDriver();
  driver_->Init(std::move(platform));
 }

void VulkanRenderProcessor::Destroy() {
  driver_->DestroySwapChain(sch_);

  driver_->Clear();
  delete driver_;
}

void VulkanRenderProcessor::End() {
}

void VulkanRenderProcessor::Start() {
  sch_ = driver_->CreateSwapChain(DriverContext::Get().window_handle_, 0);
}


void VulkanRenderProcessor::AfterRender() {

}

void VulkanRenderProcessor::BeforeRender() {

}

void VulkanRenderProcessor::Render() {

}

}  // namespace our_graph