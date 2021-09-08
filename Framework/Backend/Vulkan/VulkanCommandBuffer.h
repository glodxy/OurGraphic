//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDBUFFER_H_
#include "Framework/Backend/include_internal/ICommandBuffer.h"
#include "VulkanDef.h"
namespace our_graph {
/**
 * VulkanCommondBuffer仅用于存储自定的command，
 * 实际的buffer由CommandPool进行管理
 * */
class VulkanCommandBuffer : public ICommandBuffer {
 public:
  ~VulkanCommandBuffer()override;
  void AddCommand(std::shared_ptr<ICommand> command) override;
  bool IsAvailable() const override;
  void SetState(int state) override;
  bool IsFull() const override;
  void Clear() override;
  std::vector<std::shared_ptr<ICommand>> GetList() const override;

 private:
//  // 目前仅分为空闲与提交两个状态
//  int cur_state_{0}; // 0:idle 1:sending
  int current_state_;
  const int MAX_SIZE = 10;
  std::vector<std::shared_ptr<ICommand>> command_list_;
};
} // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDBUFFER_H_
