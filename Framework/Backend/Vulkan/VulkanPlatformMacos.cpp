//
// Created by Glodxy on 2021/8/29.
//

#include "VulkanPlatformMacos.h"
#include "VulkanDef.h"

namespace our_graph {
std::vector<const char *> VulkanPlatformMacos::GetInstanceExtLayers() const {
  static std::vector<const char*> res = {
    "VK_MVK_macos_surface"
  };
  return res;
}

void * VulkanPlatformMacos::CreateSurface(void *native_window, void *instance, uint64_t flags) {
  return nullptr;
}


}  // namespace our_graph