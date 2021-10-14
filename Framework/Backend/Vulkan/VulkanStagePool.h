//
// Created by Glodxy on 2021/10/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSTAGEPOOL_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSTAGEPOOL_H_
#include "VulkanDef.h"
#include "Framework/Backend/include/DriverEnum.h"
#include <map>
#include <unordered_set>

namespace our_graph {

struct VulkanStage {
  VmaAllocation memory;
  VkBuffer buffer;
  uint32_t capacity;
  mutable uint64_t last_accessed;
};

struct VulkanStageImage {
  VkFormat format;
  uint32_t width;
  uint32_t height;
  mutable uint64_t last_accessed;
  VmaAllocation memory;
  VkImage image;
};

/**
 * 该类用于提供内存映射，即相应的内存池给显存中的数据，
 * 换言之，即提供了内存与显存之间的通道
 * */
class VulkanStagePool {
 public:
  explicit VulkanStagePool() = default;

  const VulkanStage* AcquireStage(uint32_t num_bytes);

  const VulkanStageImage* AcquireImage(PixelDataFormat format, PixelDataType type,
                                       uint32_t width, uint32_t height);

  /**
   * 销毁所有未使用的stage
   * */
  void Reset();

  void GC();
 protected:


 private:
  /**
   * 该表用于查询未使用的stage
   * */
  std::multimap<uint32_t ,const VulkanStage*> free_stages_;
  /**
   * 存储已使用的stage
   * */
  std::unordered_set<const VulkanStage*> used_stages_;

  std::unordered_set<const VulkanStageImage*> free_images_;
  std::unordered_set<const VulkanStageImage*> used_images_;

  uint64_t current_frame_ = 0;
};
} // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSTAGEPOOL_H_
