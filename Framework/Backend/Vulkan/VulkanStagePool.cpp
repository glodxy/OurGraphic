//
// Created by Glodxy on 2021/10/2.
//

#include "VulkanStagePool.h"
#include "VulkanContext.h"
namespace {
// 以cmd buffer的数量作为最早丢弃的帧数，即在cmd buffer用完前不需要回收
static constexpr uint32_t TIME_BEFORE_EVICTION = our_graph::MAX_COMMAND_BUFFERS_COUNT;
}  // namespace

namespace our_graph {
/**
 * 获取一个供内存与显存通信的buffer通道
 * */
const VulkanStage *VulkanStagePool::AcquireStage(uint32_t num_bytes) {
  // 查找有无比请求大小大的块
  auto iter = free_stages_.lower_bound(num_bytes);
  if (iter != free_stages_.end()) {
    // 如果有,则返回
    auto stage = iter->second;
    free_stages_.erase(iter);
    used_stages_.insert(stage);
    return stage;
  }

  // 否则，创建一个资源
  VulkanStage* stage = new VulkanStage({
    .memory = VK_NULL_HANDLE,
    .buffer = VK_NULL_HANDLE,
    .capacity = num_bytes,
    .last_accessed = current_frame_,
  });

  used_stages_.insert(stage);
  // 创建一个显存的buffer，用于向其传输数据
  VkBufferCreateInfo buffer_info {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = num_bytes,
    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  };
  VmaAllocationCreateInfo alloc_info {
    .pool = VulkanContext::Get().vma_pool_cpu_,
  };
  vmaCreateBuffer(VulkanContext::Get().allocator_, &buffer_info,
                  &alloc_info, &stage->buffer, &stage->memory, nullptr);

  return stage;
}

const VulkanStageImage *VulkanStagePool::AcquireImage(PixelDataFormat format,
                                                      PixelDataType type,
                                                      uint32_t width,
                                                      uint32_t height) {
  const VkFormat vk_format = VulkanUtils::GetVkFormat(format, type);
  for (auto image : free_images_) {
    if (image->format == vk_format &&
        image->width == width &&
        image->height == height) {
      // 如果有现成的，直接用
      free_images_.erase(image);
      used_images_.insert(image);
      return image;
    }
  }

  VulkanStageImage* image = new VulkanStageImage({
    .format = vk_format,
    .width = width,
    .height = height,
    .last_accessed = current_frame_,
  });

  VkImageCreateInfo image_info {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = vk_format,
    .extent = {width, height, 1},
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_LINEAR,
    .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
             VK_IMAGE_USAGE_TRANSFER_DST_BIT,
  };
  // 该内存的作用为向GPU传输数据
  VmaAllocationCreateInfo alloc_info {
    .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
    .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
  };

  VkResult res = vmaCreateImage(VulkanContext::Get().allocator_,
                                &image_info, &alloc_info,
                                &image->image, &image->memory, nullptr);
  if (res != VK_SUCCESS) {
    LOG_ERROR("VulkanStagePool", "CreateImage Failed! code:{}", res);
  }

  return image;
}

/**
 * 清除没用的部分
 * */
void VulkanStagePool::GC() {
  // 没到最小清理帧时，略过
  if (++current_frame_ <= TIME_BEFORE_EVICTION) {
    return;
  }

  uint64_t eviction_time = current_frame_ - TIME_BEFORE_EVICTION;
  decltype(free_stages_) tmp_free_stages;
  tmp_free_stages.swap(free_stages_);
  for (auto pair : tmp_free_stages) {
    // 超过一个周期（TIME BEFORE EVICTION）未使用的资源，直接销毁
    if (pair.second->last_accessed < eviction_time) {
      vmaDestroyBuffer(VulkanContext::Get().allocator_,
                       pair.second->buffer, pair.second->memory);
      delete pair.second;
    } else {
      free_stages_.insert(pair);
    }
  }

  decltype(used_stages_) tmp_used_stages;
  tmp_used_stages.swap(used_stages_);
  for (auto stage : tmp_used_stages) {
    // 回收超过一个周期未使用的资源
    if (stage->last_accessed < eviction_time) {
      stage->last_accessed = current_frame_;
      free_stages_.insert(std::make_pair(stage->capacity, stage));
    } else {
      used_stages_.insert(stage);
    }
  }

  decltype(free_images_) tmp_free_images;
  tmp_free_images.swap(free_images_);
  for (auto image : tmp_free_images) {
    if (image->last_accessed < eviction_time) {
      vmaDestroyImage(VulkanContext::Get().allocator_, image->image,
                      image->memory);
      delete image;
    } else {
      free_images_.insert(image);
    }
  }

  decltype(used_images_) tmp_used_images;
  tmp_used_images.swap(used_images_);
  for (auto image : tmp_used_images) {
    if (image->last_accessed < eviction_time) {
      image->last_accessed = current_frame_;
      free_images_.insert(image);
    } else {
      used_images_.insert(image);
    }
  }
}

void VulkanStagePool::Reset() {
  for (auto stage : used_stages_) {
    vmaDestroyBuffer(VulkanContext::Get().allocator_, stage->buffer,
                     stage->memory);
    delete stage;
  }
  used_stages_.clear();
  for (auto pair : free_stages_) {
    vmaDestroyBuffer(VulkanContext::Get().allocator_, pair.second->buffer,
                     pair.second->memory);
    delete pair.second;
  }
  free_stages_.clear();

  for (auto image : used_images_) {
    vmaDestroyImage(VulkanContext::Get().allocator_, image->image,
                    image->memory);
    delete image;
  }
  used_images_.clear();
  for (auto image : free_images_) {
    vmaDestroyImage(VulkanContext::Get().allocator_, image->image,
                    image->memory);
    delete image;
  }
  free_images_.clear();
}
}  // namespace our_graph