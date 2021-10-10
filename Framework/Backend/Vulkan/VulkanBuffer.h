//
// Created by Glodxy on 2021/10/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBUFFER_H_
#include "VulkanDef.h"
#include "VulkanStagePool.h"

namespace our_graph {
class VulkanBuffer {
 public:
  VulkanBuffer(VulkanStagePool& stage_pool, VkBufferUsageFlags usage,
               uint32_t num_bytes);
  ~VulkanBuffer();
  void Destroy();
  void LoadFromCPU(VulkanStagePool& stage_pool,
                   const void* cpu_data, uint32_t offset_bytes,
                   uint32_t num_bytes) const;
  VkBuffer GetGPUBuffer() const {return gpu_buffer_;}

 private:
  VmaAllocation gpu_memory_ {VK_NULL_HANDLE};
  VkBuffer gpu_buffer_ {VK_NULL_HANDLE};
  VkBufferUsageFlags usage_ = {};

};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBUFFER_H_
