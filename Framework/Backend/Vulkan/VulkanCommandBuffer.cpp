//
// Created by Glodxy on 2021/8/29.
//

#include "VulkanCommandBuffer.h"
#include "Utils/OGLogging.h"

void our_graph::VulkanCommandBuffer::AddCommand(std::shared_ptr<ICommand> command) {
  command_list_.push_back(command);
}

our_graph::VulkanCommandBuffer::~VulkanCommandBuffer() noexcept {
  command_list_.clear();
}

bool our_graph::VulkanCommandBuffer::IsAvailable() const {
  return current_state_ == 0;
}

void our_graph::VulkanCommandBuffer::SetState(int state) {
  current_state_ = state;
}

bool our_graph::VulkanCommandBuffer::IsFull() const {
  return command_list_.size() < MAX_SIZE;
}

std::vector<std::shared_ptr<our_graph::ICommand>> our_graph::VulkanCommandBuffer::GetList() const {
  return command_list_;
}

void our_graph::VulkanCommandBuffer::Clear() {
  command_list_.clear();
}