//
// Created by Glodxy on 2021/9/7.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDPOOL_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDPOOL_H_
#include "../include_internal/ICommandPool.h"
#include "VulkanCommandBuffer.h"
#include "vulkan/vulkan.h"
namespace our_graph {
/**
 * 自动模式仅需调用AddCommand
 * 手动模式需自行调用Commit
 * */
class VulkanCommandPool : public ICommandPool {
 public:
  VulkanCommandPool(VkDevice device, uint32_t queue_family_idx);
  void Create() override;
  void Destroy() override;
  void AddCommand(std::shared_ptr<ICommand> command) override;

  // 手动模式 /////////////////////////////
  void AddCommand(int idx, std::shared_ptr<ICommand> command);
  /**
   * @param idx:-1时为当前项
   * */
  void Commit(int idx = -1);
 private:
  void MoveToNextBuffer();

  bool auto_control_{true};
  int current_idx_{0};
  int size_ {10};
  uint32_t queue_family_idx_;
  std::vector<VkCommandBuffer> physical_buffers_;
  VkCommandPool  pool_;
  VkDevice device_;
  std::vector<VulkanCommandBuffer> buffers_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDPOOL_H_
