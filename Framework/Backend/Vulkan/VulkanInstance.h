//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANINSTANCE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANINSTANCE_H_
#include "../IRenderInstance.h"

#include "vulkan/vulkan.h"
namespace our_graph {
class VulkanInstance : public IRenderInstance{
 public:
  VulkanInstance() = default;

  void CreateInstance() override;

  void DestroyInstance() override;

  VkInstance GetInstance() {
    return vk_instance_;
  }
 private:
  VkInstance vk_instance_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANINSTANCE_H_
