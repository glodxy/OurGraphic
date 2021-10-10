//
// Created by Glodxy on 2021/10/10.
//

#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanDef.h"

namespace our_graph {

VulkanBuffer::VulkanBuffer(VulkanStagePool &stage_pool,
                           VkBufferUsageFlags usage,
                           uint32_t num_bytes) :
                           usage_(usage){
  // 确保usage只设了1bit
  assert(usage && !(usage & usage - 1));

  VkBufferCreateInfo buffer_info {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = num_bytes,
    .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT
  };

  VmaAllocationCreateInfo alloc_info {
    .pool = VulkanContext::Get().vma_pool_gpu_,
  };
  vmaCreateBuffer(VulkanContext::Get().allocator_,
                  &buffer_info, &alloc_info,
                  &gpu_buffer_, &gpu_memory_, nullptr);
}

VulkanBuffer::~VulkanBuffer() {
  Destroy();
}

void VulkanBuffer::Destroy() {
  if (gpu_memory_ != VK_NULL_HANDLE &&
      gpu_buffer_ != VK_NULL_HANDLE) {
    vmaDestroyBuffer(VulkanContext::Get().allocator_,
                     gpu_buffer_,
                     gpu_memory_);
    gpu_buffer_ = VK_NULL_HANDLE;
    gpu_memory_ = VK_NULL_HANDLE;
  }
}

void VulkanBuffer::LoadFromCPU(VulkanStagePool &stage_pool,
                               const void *cpu_data,
                               uint32_t offset_bytes,
                               uint32_t num_bytes) const {
  const VulkanStage* stage = stage_pool.AcquireStage(num_bytes);
  void* mapped;
  // 将data拷贝到映射区域
  vmaMapMemory(VulkanContext::Get().allocator_,
               stage->memory, &mapped);
  memcpy(mapped, cpu_data, num_bytes);
  vmaUnmapMemory(VulkanContext::Get().allocator_,
                 stage->memory);
  // 上传至gpu
  vmaFlushAllocation(VulkanContext::Get().allocator_,
                     stage->memory,
                     offset_bytes,
                     num_bytes);

  const VkCommandBuffer cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;

  VkBufferCopy region {.size = num_bytes};
  vkCmdCopyBuffer(cmd_buffer, stage->buffer, gpu_buffer_, 1, &region);

  if (usage_ & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
    VkBufferMemoryBarrier barrier {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT |
          VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = gpu_buffer_,
      .size = VK_WHOLE_SIZE
    };
    vkCmdPipelineBarrier(cmd_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                         0, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (usage_ & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
    VkBufferMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT |
            VK_ACCESS_UNIFORM_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = gpu_buffer_,
        .size = VK_WHOLE_SIZE
    };
    vkCmdPipelineBarrier(cmd_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                         0, 0, nullptr, 1, &barrier, 0, nullptr);
  }
}

}  // namespace our_graph