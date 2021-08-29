//
// Created by Glodxy on 2021/8/29.
//

#include "VulkanMemoryAllocator.h"

#include "Utils/OGLogging.h"
#include "VulkanDevice.h"
#include "VulkanMemoryHandle.h"

namespace {
const uint32_t ERROR_NUM = 0xffffffff;
}

void our_graph::VulkanMemoryAllocator::Init(std::shared_ptr<IRenderDevice> device) {
  VulkanDevice* vulkan_device = dynamic_cast<VulkanDevice*>(device.get());
  device_ = vulkan_device->GetDevice();
  VkPhysicalDeviceMemoryProperties props =
      vulkan_device->GetMemoryProps();
  for (int i = 0; i < props.memoryTypeCount; ++i) {
    auto& memory_type = props.memoryTypes[i];
    if (memory_flag_indices_.find(memory_type.propertyFlags) ==
        memory_flag_indices_.end()) {
      // 不存在该flag时，加入该flag
      std::vector<uint32_t> indices;
      memory_flag_indices_[memory_type.propertyFlags] = indices;
    }
    memory_flag_indices_[memory_type.propertyFlags].push_back(memory_type.heapIndex);
  }
  LOG_INFO("Init", "VulkanMemoryAllocator Inited, indices size:{}",
           memory_flag_indices_.size());
}

void our_graph::VulkanMemoryAllocator::Clear() {
  memory_flag_indices_.clear();
  std::vector<std::string> keys;
  keys.reserve(memory_map_.size());
  for (auto& pair : memory_map_) {
    keys.push_back(pair.first);
  }

  for (auto& key : keys) {
    DestroyGPUMemory(key);
  }
  memory_map_.clear();
  LOG_INFO("Clear", "VulkanMemoryAllocator Destroyed!");
}

std::shared_ptr<our_graph::IMemoryHandle> our_graph::VulkanMemoryAllocator::AllocateGPUMemory(const std::string &name,
                                                                                   uint64_t size) {
  VkMemoryAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.pNext = nullptr;
  allocate_info.allocationSize = size;
  // todo：此处先暂时只选择可映射的
  allocate_info.memoryTypeIndex = GetTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  VkDeviceMemory* memory = new VkDeviceMemory();
  vkAllocateMemory(device_, &allocate_info, nullptr, memory);
  std::shared_ptr<IMemoryHandle> memory_handle =
      std::make_shared<VulkanMemoryHandle>(memory, name, size);
  if (memory_map_.find(name) != memory_map_.end()) {
    //表内已有该name的memory，则报错
    LOG_ERROR("AllocateGPUMemory", "allocate gpu memory failed! id[{}] existed!", name);
    return nullptr;
  }
  memory_map_[name] = memory_handle;
  LOG_INFO("AllocateGPUMemory", "allocated {} bytes to [{}]",
           size, name);
  return memory_handle;
}

void our_graph::VulkanMemoryAllocator::DestroyGPUMemory(const std::string &name) {
  if (memory_map_.find(name) == memory_map_.end()) {
    LOG_ERROR("DestroyGPUMemory", "no gpu memory [{}]!", name);
    return;
  }
  auto iter = memory_map_.find(name);
  if (iter->second.use_count() > 1) {
    LOG_ERROR("DestroyGPUMemory", "may try to destroy memory[{}] while using!", name);
    return;
  }
  VulkanMemoryHandle* memory_handle =
      dynamic_cast<VulkanMemoryHandle*>(iter->second.get());
  VkDeviceMemory* memory = memory_handle->GetMemory();
  vkFreeMemory(device_, *memory, nullptr);

  memory_map_.erase(iter);
  LOG_INFO("DestroyGPUMemory", "destroyed memory [{}]", name);
}

bool our_graph::VulkanMemoryAllocator::MapGPUMemoryToLocal(const std::string &name, void **p_local) {
  if (memory_map_.find(name) == memory_map_.end()) {
    LOG_ERROR("MapGPUMemoryToLocal", "no gpu memory [{}]!", name);
    return false;
  }
  auto& iter = memory_map_.find(name);
  VulkanMemoryHandle* memory_handle =
      dynamic_cast<VulkanMemoryHandle*>(iter->second.get());
  VkDeviceMemory* memory = memory_handle->GetMemory();
  uint64_t size = memory_handle->GetSize();
  //todo: 添加映射检查，避免映射到已映射的位置范围内
  vkMapMemory(device_, *memory, 0, size, 0, p_local);
  return true;
}

bool our_graph::VulkanMemoryAllocator::UnMapGPUMemory(const std::string &name) {
  if (memory_map_.find(name) == memory_map_.end()) {
    LOG_ERROR("UnMapGPUMemory", "no gpu memory [{}]!", name);
    return false;
  }
  auto iter = memory_map_.find(name);
  VulkanMemoryHandle* memory_handle =
      dynamic_cast<VulkanMemoryHandle*>(iter->second.get());
  VkDeviceMemory* memory = memory_handle->GetMemory();
  vkUnmapMemory(device_, *memory);
  return true;
}


uint32_t our_graph::VulkanMemoryAllocator::GetTypeIndex(VkMemoryPropertyFlags flag) const {
  uint64_t key = flag;
  for (auto& pair : memory_flag_indices_) {
    if ((pair.first & key) != 0) {
      //目前为找到的第一个
      //todo:优化选择策略
      return  pair.second.front();
    }
  }
  return ERROR_NUM;
}