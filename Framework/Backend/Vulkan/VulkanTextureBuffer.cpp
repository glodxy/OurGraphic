//
// Created by Glodxy on 2021/9/6.
//

#include "VulkanTextureBuffer.h"

#include "VulkanMemoryAllocator.h"
#include "VulkanMemoryHandle.h"
#include "Utils/OGLogging.h"

our_graph::VulkanTextureBuffer::VulkanTextureBuffer(
    const std::string& name,
    VkDevice device,
    VkImageCreateInfo create_info,
    uint64_t memory_flag_bits) : IBuffer() {
  name_ = name;
  device_ = device;
  create_info_ = create_info;
  flag_bits_ = memory_flag_bits;
  Create();
}

our_graph::VulkanTextureBuffer::VulkanTextureBuffer(
    const std::string &name,
    VkDevice device,
    VkImage image,
    bool need_buffer) :
    name_(name) {
  device_ = device;
  image_ = image;
  has_buffer_ = need_buffer;
  if (need_buffer) {
    memory_ =
        MemoryAllocator::Get<VulkanMemoryAllocator>()->GetMemory<VulkanMemoryHandle>(name);
    vkBindImageMemory(device_, image_, *((VkDeviceMemory *) memory_->GetMemory()), 0);
  }
}

void our_graph::VulkanTextureBuffer::Create() {
  // 创建image
  if (!CreateImage()) {
    LOG_ERROR("VulkanTexture", "CreateImage failed!");
    return;
  }
  // 分配显存
  if(!AllocateMemory()) {
    LOG_ERROR("VulkanTexture", "Allocate ImageMemory failed!");
    return;
  }
  // 绑定显存
  vkBindImageMemory(device_, image_, *((VkDeviceMemory*) memory_->GetMemory()), 0);
  LOG_INFO("VulkanTexture", "CreateTexture:{}", name_);
}


bool our_graph::VulkanTextureBuffer::CreateImage() {
  VkResult image_create_res =
      vkCreateImage(device_, &create_info_,
                    nullptr, &image_);
  if (image_create_res != VK_SUCCESS) {
    LOG_ERROR("CreateImage", "create image failed!"
                             "res:{}", image_create_res);
    return false;
  }


  return true;
}

bool our_graph::VulkanTextureBuffer::AllocateMemory() {
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(device_, image_, &memory_requirements);
  uint32_t bits = memory_requirements.memoryTypeBits; // 类型的bits
  uint64_t size = memory_requirements.size;
  int idx = 0;
  for(int i = 0; i < 32; ++i) {
    if ((memory_requirements.memoryTypeBits &
        (1<<i)) != 0) {
      VkMemoryType memory_type =
          MemoryAllocator::Get<VulkanMemoryAllocator>()->GetMemoryTypeByIdx(i);
      // 如果flag为0，则必定满足
      if ((memory_type.propertyFlags & flag_bits_) == flag_bits_) {
        idx = memory_type.heapIndex;
        break;
      }
    }
  }
  if (idx >= 32) {
    LOG_ERROR("AllocateMemory", "texture allocate memory failed!"
                                "cannot find available memory for"
                                "bits:{}, size:{}", bits, size);
    return false;
  }

  std::shared_ptr<MemoryHandle> memory_handle  =
      MemoryAllocator::Get<VulkanMemoryAllocator>()->AllocateGPUMemoryByIdx(name_, size, idx);

  if (!memory_handle) {
    LOG_ERROR("AllocateMemory", "texture allocate memory failed! "
                                "Allocate GPU Memory failed!");
    return false;
  }
  memory_ = memory_handle;
  return true;
}

our_graph::VulkanTextureBuffer::~VulkanTextureBuffer() noexcept {
  // 没有buffer时，代表为外部创建的资源，在外部自行销毁
  if (has_buffer_) {
    vkDestroyImage(device_, image_, nullptr);
    image_ = VK_NULL_HANDLE;
  }
  memory_ = nullptr;
}

void *our_graph::VulkanTextureBuffer::GetInstance() {
  return &image_;
}