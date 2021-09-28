//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPLATFORMWINDOWS_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPLATFORMWINDOWS_H_


#include "../include_internal/IPlatform.h"

namespace our_graph {
class VulkanPlatformWindows : public IPlatform{
 public:
  std::vector<const char *> GetInstanceExtLayers() const override;

  bool CreateSurface(void *native_window, void *instance, uint64_t flags, void* surface) override;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPLATFORMWINDOWS_H_
