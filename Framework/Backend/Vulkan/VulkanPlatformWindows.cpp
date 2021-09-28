//
// Created by Glodxy on 2021/8/29.
//

#include "VulkanPlatformWindows.h"
#include "VulkanDef.h"

std::vector<const char *> our_graph::VulkanPlatformWindows::GetInstanceExtLayers() const {
  static std::vector<const char*> res = {
      "VK_KHR_win32_surface"
  };
  return res;
}

void * our_graph::VulkanPlatformWindows::CreateSurface(void *native_window, void *instance, uint64_t flags) {
  VkSurfaceKHR surface;

  HWND window = (HWND)native_window;
  VkWin32SurfaceCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.hwnd = window;
  create_info.hinstance = GetModuleHandle(nullptr);

  VkResult result = vkCreateWin32SurfaceKHR((VkInstance) instance, &create_info, nullptr, &surface);
  if (result != VK_SUCCESS) {
    LOG_ERROR("VulkanPlatformWindows", "CreateSurface Failed! code:{}", result);
    return nullptr;
  }

  return surface;
}