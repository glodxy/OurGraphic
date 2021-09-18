//
// Created by Glodxy on 2021/8/29.
//

#include "VulkanCommandBuffer.h"
#include "Utils/OGLogging.h"

namespace {
const int STATE_IDLE = 0;
const int STATE_USE = 1;
const int STATE_ARCHIVE = 2;
}

our_graph::VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer buffer) : buffer_(buffer){
  VkFenceCreateInfo fenceCreateInfo { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  VkDevice device = *(VulkanContext::Get().device_);
  device_ = device;
  vkCreateFence(device, &fenceCreateInfo, nullptr, &fence_);
}

void our_graph::VulkanCommandBuffer::AddCommand(std::shared_ptr<ICommand> command) {
  command_list_.push_back(command);
}

our_graph::VulkanCommandBuffer::~VulkanCommandBuffer() noexcept {
  vkDestroyFence(device_, fence_, nullptr);
  command_list_.clear();
}

bool our_graph::VulkanCommandBuffer::IsAvailable() const {
  return current_state_ == 0;
}

void our_graph::VulkanCommandBuffer::SetState(int state) {
  current_state_ = state;
}

void our_graph::VulkanCommandBuffer::Use() {
  SetState(STATE_USE);
  const VkCommandBufferBeginInfo begin_info {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(buffer_, &begin_info);
}

void our_graph::VulkanCommandBuffer::EndUse() {
  vkEndCommandBuffer(buffer_);


  VkSubmitInfo submit_info {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = nullptr,
      .pWaitDstStageMask = nullptr,
      .commandBufferCount = 1,
      .pCommandBuffers = &buffer_,
      .signalSemaphoreCount = 0,
      .pSignalSemaphores =nullptr,
  };
  VkQueue queue = *(VulkanContext::Get().graphic_queue_);
  VkResult res = vkQueueSubmit(queue, 1, &submit_info, fence_);
  //CHECK_RESULT(res, "VulkanCommandBuffer", "Submit Failed!");

  SetState(STATE_ARCHIVE);
  LOG_INFO("VulkanCommandBuffer", "START WAIT-----------");
  res = vkQueueWaitIdle(queue);
  LOG_INFO("VulkanCommandBuffer", "FINISHED-----------");
  SetState(STATE_IDLE);
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

void * our_graph::VulkanCommandBuffer::GetInstance() {
  return &buffer_;
}