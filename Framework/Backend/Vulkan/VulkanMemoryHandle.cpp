//
// Created by Glodxy on 2021/8/29.
//

#include "VulkanMemoryHandle.h"

our_graph::VulkanMemoryHandle::VulkanMemoryHandle(
    VkDeviceMemory *memory,
    const std::string &name,
    uint64_t size) : memory_(memory){
  memory_name_ = name;
  memory_size_ = size;
}