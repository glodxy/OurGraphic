//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDBUFFER_H_
#include "Framework/Backend/include/ICommandBuffer.h"

namespace our_graph {
class VulkanCommandBuffer : public ICommandBuffer{
 public:
  void Create() override;
  void Destroy() override;
};
} // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANCOMMANDBUFFER_H_
