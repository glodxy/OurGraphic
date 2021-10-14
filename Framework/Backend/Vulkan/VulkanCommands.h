//
// Created by Glodxy on 2021/9/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDS_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDS_H_

#include "VulkanDef.h"
#include "Framework/Backend/include/DriverEnum.h"
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
  std::shared_ptr<VulkanCmdFence> fence_;
  uint32_t index = 0; // 该变量用于标识该cmd buffer对应的序号
};

class CommandBufferObserver {
 public:
  virtual void OnCommandBuffer(const VulkanCommandBuffer& cmd_buffer) = 0;
  virtual ~CommandBufferObserver();
};

/**
 * 以懒加载的方式给外部提供cmd buffer
 * */
class VulkanCommands {
 public:
  VulkanCommands(VkDevice device, uint32_t queue_family_idx);
  ~VulkanCommands();

  /**
   * 提供不可改变的cmd buffer给外部使用
   * */
  const VulkanCommandBuffer& Get();

  /**
   * 如果当前的cmd buffer存在且有要提交的东西，则提交并设为空
   * */
  bool Commit();

  /**
   * 返回渲染帧是否结束的信号量
   * 用于在展示后移除对当前cmd buffer的依赖
   * */
  VkSemaphore AcquireFinishedSignal();

  /**
   * 获取一个信号量判断能否进行下一次提交
   * */
  void InjectDependency(VkSemaphore next);

  void Wait();

  void UpdateFence();

  void SetObserver(CommandBufferObserver* observer) {
    observer_ = observer;
  }

  void GC();
 protected:

 private:
  static constexpr int CAPACITY = MAX_COMMAND_BUFFERS_COUNT;
  const VkDevice device_;
  VkQueue queue_;
  VkCommandPool pool_;
  VulkanCommandBuffer* current_ = nullptr;
  VulkanCommandBuffer storage_[CAPACITY] = {};
  VkSemaphore submit_signals_[CAPACITY] = {};

  VkSemaphore submit_signal_ = {};
  VkSemaphore injected_signal_ = {};

  size_t available_cnt_ = CAPACITY;
  CommandBufferObserver* observer_ = nullptr;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDS_H_
