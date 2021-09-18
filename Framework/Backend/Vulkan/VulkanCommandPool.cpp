//
// Created by Glodxy on 2021/9/7.
//

#include "VulkanCommandPool.h"

#include "VulkanCommandBuffer.h"
#include "Utils/OGLogging.h"

our_graph::VulkanCommandPool::VulkanCommandPool(VkDevice device, uint32_t queue_family_idx) {
  device_ = device;
  queue_family_idx_ = queue_family_idx;
  Create();
}


void our_graph::VulkanCommandPool::Create() {
  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  create_info.queueFamilyIndex = queue_family_idx_;

  vkCreateCommandPool(device_, &create_info, nullptr, &pool_);

  // 设置逻辑指令缓存
  buffers_.reserve(size_);

  // 创建物理指令缓存
  physical_buffers_.resize(size_);
  VkCommandBufferAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.pNext = nullptr;
  allocate_info.commandPool = pool_;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = size_;
  vkAllocateCommandBuffers(device_, &allocate_info, physical_buffers_.data());

  for (auto& buffer: physical_buffers_) {
    buffers_.emplace_back(buffer);
  }
}

void our_graph::VulkanCommandPool::AddCommand(std::shared_ptr<ICommand> command) {
  buffers_[current_idx_].AddCommand(command);

  // 此处自动提交判断command数量以切换buffer
  if (auto_control_) {
    // 如果当前已满则换至下一个可用buffer
    // 并将当前设为发送状态
    if (buffers_[current_idx_].IsFull()) {
      buffers_[current_idx_].SetState(1);
      MoveToNextBuffer();
      // todo:提交至GPU
    }
  } else {
    // 手动模式, 只是简单的将其添入buffer
    LOG_INFO("VulkanCommandPool", "manual add command");
  }
}

void our_graph::VulkanCommandPool::AddCommand(int idx, std::shared_ptr<ICommand> command) {
  if (auto_control_) {
    LOG_ERROR("VulkanCommandPool", "AddCommand failed! not manual mode!");
    return;
  }
  buffers_[idx].AddCommand(command);
}

void our_graph::VulkanCommandPool::Commit(int idx) {
  if (auto_control_) {
    LOG_ERROR("VulkanCommandPool", "AddCommand failed! not manual mode!");
    return;
  }
  // 提交当前的
  if (idx == -1 || current_idx_ == idx) {
    // 设置状态
    buffers_[current_idx_].SetState(1);
    //todo:提交至GPU

    // 将current设至下一个
    MoveToNextBuffer();
  } else {
    // 设置状态
    buffers_[idx].SetState(1);
    //todo:提交至GPU
  }
}

void our_graph::VulkanCommandPool::MoveToNextBuffer() {
  // 查找下一个可用的
  int i = ((current_idx_ + 1) % size_);
  while (i != current_idx_) {
    if (buffers_[i].IsAvailable()) {
      break;
    }
    i = ((i + 1) % size_);
  }
  if (i == current_idx_) {
    LOG_ERROR("VulkanCommandPool", "cannot find available buffer!");
    return;
  }
  // 找到即设置当前的idx为i
  current_idx_ = i;
}

void our_graph::VulkanCommandPool::Destroy() {
  buffers_.clear();
  vkFreeCommandBuffers(device_, pool_, size_, physical_buffers_.data());
  vkDestroyCommandPool(device_, pool_, nullptr);
}

our_graph::ICommandBuffer * our_graph::VulkanCommandPool::GetBuffer() {
  MoveToNextBuffer();
  if (!buffers_[current_idx_].IsAvailable()) {
    return nullptr;
  }
  buffers_[current_idx_].Use();
  return &(buffers_[current_idx_]);
}