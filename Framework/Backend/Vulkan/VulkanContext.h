//
// Created by Glodxy on 2021/10/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCONTEXT_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCONTEXT_H_
#include "VulkanDef.h"
#include "VulkanCommands.h"

namespace our_graph {

struct VulkanRenderTarget;
struct VulkanSwapChain;
struct VulkanTexture;
class VulkanStagePool;

// TODO: make this as lean as possible, it makes VulkanRenderTarget very big (currently 880 bytes).
struct VulkanAttachment {
  VkFormat format;
  VkImage image {VK_NULL_HANDLE};
  VkImageView view {VK_NULL_HANDLE};
  VkDeviceMemory memory {VK_NULL_HANDLE};
  VulkanTexture* texture = nullptr;
  VkImageLayout layout;
  uint8_t level;
  uint16_t layer;
};

struct VulkanTimestamps {
  VkQueryPool pool;
  uint32_t used;
  utils::Mutex mutex;
};

struct VulkanRenderPass {
  VkRenderPass renderPass;
  uint32_t subpassMask;
  int currentSubpass;
  VulkanTexture* depthFeedback;
};

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
  VkInstance instance_{VK_NULL_HANDLE};
  uint32_t graphic_queue_family_idx_{0};
  VkQueue* graphic_queue_ {nullptr};
  VulkanCommands* commands_ {nullptr};
  VmaAllocator allocator_;
  VmaPool vma_pool_cpu_ {nullptr}; // cpu的内存池
  VmaPool vma_pool_gpu_ {nullptr}; // gpu的内存池
  VkFormat final_depth_format_ {};
  VulkanTexture* empty_texture {nullptr};
 private:
  VulkanContext() = default;

};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCONTEXT_H_
