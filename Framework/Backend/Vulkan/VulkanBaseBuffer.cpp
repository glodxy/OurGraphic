//
// Created by Glodxy on 2021/9/19.
//

#include "VulkanBaseBuffer.h"
#include "VulkanMemoryAllocator.h"
#include "VulkanMemoryHandle.h"
/**
 * 从现有buffer创建
 * */
our_graph::VulkanBaseBuffer::VulkanBaseBuffer(
    const std::string &name,
    VkDevice device,
    VkBuffer buffer,
    bool auto_control) :
    device_(device),
    buffer_(buffer),
    name_(name),
    auto_control_(auto_control) {
  if (auto_control_) {
    memory_ =
        MemoryAllocator::Get<VulkanMemoryAllocator>()->GetMemory<VulkanMemoryHandle>(name);
    vkBindBufferMemory(device_, buffer_, *((VkDeviceMemory *) memory_->GetMemory()), 0);
  }
}

our_graph::VulkanBaseBuffer::VulkanBaseBuffer(
    const std::string &name,
    VkDevice device,
    VkBufferCreateInfo create_info,
    uint64_t memory_flag_bits) :
    device_(device),
    buffer_create_info_(create_info),
    flag_bits_(memory_flag_bits) {
  Create();
}

void our_graph::VulkanBaseBuffer::Create() {
  if (!CreateBuffer()) {
    LOG_ERROR("VulkanBaseBuffer", "CreateBuffer Failed!");
    return;
  }

  if (!AllocateMemory()) {
    LOG_ERROR("VulkanBaseBuffer", "AllocateMemory Failed!");
    return;
  }

  // 此处待定，现只在写入数据时进行映射
//  if (!MapMemory()) {
//    LOG_ERROR("VulkanBaseBuffer", "MapMemory Failed!");
//    return;
//  }

  // 绑定显存
  vkBindBufferMemory(device_, buffer_, *((VkDeviceMemory *) memory_->GetMemory()), 0);
}

bool our_graph::VulkanBaseBuffer::CreateBuffer() {
  VkResult result = vkCreateBuffer(
      device_, &buffer_create_info_, nullptr, &buffer_);
  if (result != VK_SUCCESS) {
    LOG_ERROR("CreateImage", "create image failed!"
                             "res:{}", result);
    return false;
  }
  buffer_descriptor_info_.buffer = buffer_;
  return true;
}

bool our_graph::VulkanBaseBuffer::AllocateMemory() {
  // 查询需要的显存需求
  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(device_, buffer_, &memory_requirements);
  uint32_t bits = memory_requirements.memoryTypeBits; // 类型的bits
  uint64_t size = memory_requirements.size;

  buffer_descriptor_info_.range = size;
  buffer_descriptor_info_.offset = 0;

  std::shared_ptr<MemoryHandle> memory_handle  =
     MemoryAllocator::Get<VulkanMemoryAllocator>()->AllocateGPUMemoryByType(
         name_, size, bits);

  if (!memory_handle) {
    LOG_ERROR("VulkanBaseBuffer", "buffer allocate memory failed! "
                                "Allocate GPU Memory failed!");
    return false;
  }
  memory_ = memory_handle;
  local_memory_ = new uint8_t[size];
  if (!local_memory_) {
    LOG_ERROR("VulkanBaseBuffer", "allocate local memory failed! size:{}", size);
    return false;
  }
  return true;
}

bool our_graph::VulkanBaseBuffer::WriteData(void *src, int size) {
  MapMemory();
  memcpy(local_memory_, src, size);
  UnMapMemory();
  return true;
}

bool our_graph::VulkanBaseBuffer::MapMemory() {
  is_mapping_ = true;
  // todo:1.先从stage pool分配对应size，并获取句柄
  // todo:2.将显存映射至该句柄对应的内存
  MemoryAllocator::Get<VulkanMemoryAllocator>()->MapGPUMemoryToLocal(
      name_, (void**)&local_memory_);
  return true;
}

bool our_graph::VulkanBaseBuffer::UnMapMemory() {
  // 此处只结束显存与内存的映射
  // 内存依然持有数据，stage pool不进行回收
  MemoryAllocator::Get<VulkanMemoryAllocator>()->UnMapGPUMemory(
      name_);
  is_mapping_ = false;
  return true;
}

void *our_graph::VulkanBaseBuffer::GetInstance() {
  return &buffer_;
}

void* our_graph::VulkanBaseBuffer::GetLocalMemory() {
  return local_memory_;
}

our_graph::VulkanBaseBuffer::~VulkanBaseBuffer() noexcept {
  // 如果存在映射，销毁该映射
  if (is_mapping_) {
    UnMapMemory();
  }

  // todo:改为stage pool 回收
  if (local_memory_) {
    delete[] local_memory_;
    local_memory_ = nullptr;
  }

  // 非auto，代表为外部创建的资源，在外部自行销毁
  if (auto_control_) {
    vkDestroyBuffer(device_, buffer_, nullptr);
    buffer_ = VK_NULL_HANDLE;
  }
  memory_ = nullptr;
}