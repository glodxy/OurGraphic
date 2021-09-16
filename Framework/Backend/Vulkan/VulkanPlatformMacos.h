//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPLATFORMMACOS_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPLATFORMMACOS_H_

#include "../include_internal/IPlatform.h"

namespace our_graph {
class VulkanPlatformMacos : public IPlatform{
 public:
  std::vector<const char *> GetInstanceExtLayers() const override;

  void * CreateSurface(void *native_window, void *instance, uint64_t flags) override;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANPLATFORMMACOS_H_
