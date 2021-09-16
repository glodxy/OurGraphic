//
// Created by Glodxy on 2021/8/29.
//

#include "VulkanPlatformWindows.h"

std::vector<const char *> our_graph::VulkanPlatformWindows::GetInstanceExtLayers() const {
  static std::vector<const char*> res = {
      "VK_KHR_win32_surface"
  };
  return res;
}

void * our_graph::VulkanPlatformWindows::CreateSurface(void *native_window, void *instance, uint64_t flags) {
  return nullptr;
}