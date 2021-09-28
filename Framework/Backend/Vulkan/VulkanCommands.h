//
// Created by Glodxy on 2021/9/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDS_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDS_H_

#include "VulkanDef.h"
#include "Utils/Mutex.h"

namespace our_graph {
struct VulkanCmdFence {
  VulkanCmdFence(VkDevice device, bool signal = false);
  ~VulkanCmdFence();

  const VkDevice device_;
  VkFence fence_;
  utils::Condition condition_;
  utils::Mutex  mutex_;
  std::atomic<VkResult> status_;
};

// 该类作为控制使用的句柄实例
struct VulkanCommandBuffer {
  VulkanCommandBuffer() {}
  // 禁用拷贝
  VulkanCommandBuffer(const VulkanCommandBuffer& r) = delete;
  VulkanCommandBuffer& operator=(VulkanCommandBuffer const&) = delete;

  VkCommandBuffer cmd_buffer_ {VK_NULL_HANDLE};
};

class CommandBufferObserver {
 public:
  virtual void OnCommandBuffer(const VulkanCommandBuffer& cmd_buffer) = 0;
  virtual ~CommandBufferObserver();
};

class VulkanCommands {

};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDS_H_
