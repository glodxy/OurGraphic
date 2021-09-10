//
// Created by Glodxy on 2021/9/8.
//

#include "VulkanSwapChain.h"

bool our_graph::VulkanSwapChain::CreateSwapChainExt() {

  INSTANCE_FUNC_PTR(instance_, GetPhysicalDeviceSurfaceSupportKHR);
  CHECK_FUNC_PTR_RETURN(GetPhysicalDeviceSurfaceSupportKHR, false);

  INSTANCE_FUNC_AND_CHECK_RETURN(instance_, GetPhysicalDeviceSurfaceCapabilitiesKHR, false);
  INSTANCE_FUNC_AND_CHECK_RETURN(instance_, GetPhysicalDeviceSurfaceFormatsKHR, false);
  INSTANCE_FUNC_AND_CHECK_RETURN(instance_, GetPhysicalDeviceSurfacePresentModesKHR, false);
  INSTANCE_FUNC_AND_CHECK_RETURN(instance_, DestroySurfaceKHR, false);

  DEVICE_FUNC_AND_CHECK_RETURN(device_, CreateSwapchainKHR, false);
  DEVICE_FUNC_AND_CHECK_RETURN(device_, DestroySwapchainKHR, false);
  DEVICE_FUNC_AND_CHECK_RETURN(device_, GetSwapchainImagesKHR, false);
  DEVICE_FUNC_AND_CHECK_RETURN(device_, AcquireNextImageKHR, false);
  DEVICE_FUNC_AND_CHECK_RETURN(device_, QueuePresentKHR, false);

  return true;
}

bool our_graph::VulkanSwapChain::CreateSurface(
    WindowInstance ins, WindowHandle handle) {
  VkWin32SurfaceCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.pNext = nullptr;
  create_info.hinstance = ins;
  create_info.hwnd = handle;

  VkResult res = vkCreateWin32SurfaceKHR(instance_, &create_info, nullptr, &surface_);
  if (res != VK_SUCCESS) {
    LOG_ERROR("VulkanSwapchain", "create swapchain failed!");
    return false;
  }
  return true;
}