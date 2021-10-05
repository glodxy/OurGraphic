//
// Created by Glodxy on 2021/10/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCONTEXT_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCONTEXT_H_
#include "VulkanDef.h"
#include "VulkanCommands.h"

namespace our_graph {
class VulkanContext {
 public:
  static VulkanContext& Get() {
    static VulkanContext context_;
    return context_;
  }
  VkPhysicalDevice* physical_device_{nullptr};
  VkPhysicalDeviceProperties physical_device_properties_{};
  VkPhysicalDeviceMemoryProperties memory_properties_{};
  VkDevice* device_ {nullptr};
  uint32_t graphic_queue_family_idx_{0};
  VkQueue* graphic_queue_ {nullptr};
  VulkanCommands* commands_ {nullptr};
  VmaAllocator allocator_;
  VmaPool vma_pool_cpu_ {nullptr}; // cpu的内存池
  VmaPool vma_pool_gpu_ {nullptr}; // gpu的内存池
  VkFormat final_depth_format_ {};
 private:
  VulkanContext() = default;

};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCONTEXT_H_
