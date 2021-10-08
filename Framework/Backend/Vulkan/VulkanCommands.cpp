//
// Created by Glodxy on 2021/9/26.
//

#include "VulkanCommands.h"
#include "../include_internal/DriverEnum.h"
#include "VulkanContext.h"

namespace our_graph {

VulkanCmdFence::VulkanCmdFence(VkDevice device, bool signal) : device_(device){
  VkFenceCreateInfo fence_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
  };
  if (signal) {
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  }
  vkCreateFence(device_, &fence_info, nullptr, &fence_);

  status_.store(VK_INCOMPLETE);
}

VulkanCmdFence::~VulkanCmdFence() {
  vkDestroyFence(device_, fence_, nullptr);
}

CommandBufferObserver::~CommandBufferObserver() {}

VulkanCommands::VulkanCommands(VkDevice device, uint32_t queue_family_idx) : device_(device){
  VkCommandPoolCreateInfo pool_create_info = {};
  pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_create_info.flags =
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  pool_create_info.queueFamilyIndex = queue_family_idx;
  vkCreateCommandPool(device_, &pool_create_info, nullptr, &pool_);
  vkGetDeviceQueue(device_, queue_family_idx, 0, &queue_);

  VkSemaphoreCreateInfo semaphore_create_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  for (auto& semaphore : submit_signals_) {
    vkCreateSemaphore(device_, &semaphore_create_info, nullptr, &semaphore);
  }

  for (int i = 0; i < MAX_COMMAND_BUFFERS_COUNT; ++i) {
    storage_[i].index = i;
  }
}

VulkanCommands::~VulkanCommands() {
  Wait();
  GC();
  vkDestroyCommandPool(device_, pool_, nullptr);
  for (auto semaphore : submit_signals_) {
    vkDestroySemaphore(device_, semaphore, nullptr);
  }
}

const VulkanCommandBuffer &VulkanCommands::Get() {
  if (current_) {
    return *current_;
  }

  /**
   * 当没有可用的cmd buffer时，等待
   * */
   while (available_cnt_ == 0) {
     LOG_INFO("VulkanCommandBuffer", "Wait For Available CommandBuffer");


     Wait();
     GC();
   }

   // 找到可用的cmd buffer，设至当前位置
   for (VulkanCommandBuffer& wrapper : storage_) {
     if (wrapper.cmd_buffer_ == VK_NULL_HANDLE) {
       current_ = &wrapper;
       break;
     }
   }

   assert(current_ != nullptr);
   --available_cnt_;

   VkCommandBufferAllocateInfo allocate_info {
     .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
     .commandPool = pool_,
     .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
     .commandBufferCount = 1
   };
   vkAllocateCommandBuffers(device_, &allocate_info, &current_->cmd_buffer_);

   current_->fence_ = std::make_shared<VulkanCmdFence>(device_);

   VkCommandBufferBeginInfo begin_info {
     .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
     .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
   };
  vkBeginCommandBuffer(current_->cmd_buffer_, &begin_info);

  // 通知当前cmd buffer发生了变更
  if (observer_) {
    observer_->OnCommandBuffer(*current_);
  }

  return *current_;
}

bool VulkanCommands::Commit() {
  if (!current_) {
    return false;
  }

  const int64_t index = current_ - &storage_[0];
  VkSemaphore render_finished = submit_signals_[index];

  vkEndCommandBuffer(current_->cmd_buffer_);

  //todo:解释
  VkPipelineStageFlags wait_dest_stage_masks[2] = {
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
  };

  VkSemaphore signals[2] = {
      VK_NULL_HANDLE,
      VK_NULL_HANDLE,
  };

  VkSubmitInfo submit_info {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 0,
    .pWaitSemaphores = signals,
    .pWaitDstStageMask = wait_dest_stage_masks,
    .commandBufferCount = 1,
    .pCommandBuffers = &current_->cmd_buffer_,
    .signalSemaphoreCount = 1u,
    .pSignalSemaphores = &render_finished,
  };

  // 如果当前有提交，则需先等待该提交
  if (submit_signal_) {
    signals[submit_info.waitSemaphoreCount++] = submit_signal_;
  }

  // todo：
  // 仍需等待该信号量
  if (injected_signal_) {
    signals[submit_info.waitSemaphoreCount++] = injected_signal_;
  }

  LOG_INFO("VulkanCommandBuffer", "Submit CommandBuffer:\n"
                                  "wait ({}, {})\n"
                                  "signal = {}",
           (void*)(signals[0]), (void*)(signals[1]),
           (void*)render_finished);

  auto& cmd_fence = current_->fence_;
  // 多线程上锁
  std::unique_lock<utils::Mutex> lock(cmd_fence->mutex_);
  cmd_fence->status_.store(VK_NOT_READY);
  VkResult res = vkQueueSubmit(queue_, 1, &submit_info, cmd_fence->fence_);
  if (res != VK_SUCCESS) {
    LOG_ERROR("VulkanCommandBuffer", "SubmitCommand Failed!");
  }
  cmd_fence->condition_.notify_all();
  lock.unlock();

  submit_signal_ = render_finished;
  injected_signal_ = VK_NULL_HANDLE;
  current_ = nullptr;
  return true;
}

VkSemaphore VulkanCommands::AcquireFinishedSignal() {
  VkSemaphore semaphore = submit_signal_;
  submit_signal_ = VK_NULL_HANDLE;
  LOG_INFO("VulkanCommandBuffer", "Acquire {} for present!",
           (void*)semaphore);
  return semaphore;
}

void VulkanCommands::InjectDependency(VkSemaphore next) {
  injected_signal_ = next;
  LOG_INFO("VulkanCommandBuffer", "Injecting {}!", (void*)next);
}

void VulkanCommands::Wait() {
  VkFence fences[CAPACITY];
  uint32_t cnt = 0;
  int i = 0;
  for (VulkanCommandBuffer& wrapper : storage_) {
    ++i;
    // 当前没在使用该cmd buffer且其存在
    if (wrapper.cmd_buffer_ != VK_NULL_HANDLE && current_ != &wrapper) {
      fences[cnt++] = wrapper.fence_->fence_;
    }
  }
  if (cnt > 0) {
    vkWaitForFences(device_, cnt, fences, VK_TRUE, UINT64_MAX);
  }
}

void VulkanCommands::GC() {
  for (VulkanCommandBuffer& wrapper : storage_) {
    if (wrapper.cmd_buffer_ != VK_NULL_HANDLE) {
      VkResult res = vkWaitForFences(device_, 1, &wrapper.fence_->fence_, VK_TRUE, 0);
      if (res == VK_SUCCESS) {
        vkFreeCommandBuffers(device_, pool_, 1, &wrapper.cmd_buffer_);
        wrapper.cmd_buffer_ = VK_NULL_HANDLE;
        wrapper.fence_->status_.store(VK_SUCCESS);
        wrapper.fence_.reset();
        ++available_cnt_;
      }
    }
  }
}

void VulkanCommands::UpdateFence() {
  for (VulkanCommandBuffer& wrapper : storage_) {
    if (wrapper.cmd_buffer_ != VK_NULL_HANDLE) {
      VulkanCmdFence* fence = wrapper.fence_.get();
      if (fence) {
        VkResult status = vkGetFenceStatus(device_, fence->fence_);
        fence->status_.store(status, std::memory_order_relaxed);
      }
    }
  }
}

}  // namespace our_graph