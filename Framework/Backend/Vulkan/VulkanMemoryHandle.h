//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYHANDLE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYHANDLE_H_
#include "../IMemoryHandle.h"
#include "vulkan/vulkan.h"
namespace our_graph {
/**
 * vulkan的显存句柄
 * */
class VulkanMemoryHandle : public IMemoryHandle{
 public:
  explicit VulkanMemoryHandle(VkDeviceMemory* memory,
                              const std::string& name,
                              uint64_t size);
  VkDeviceMemory* GetMemory() {
    return memory_;
  }

  ~VulkanMemoryHandle() override{
    delete memory_;
    memory_ = nullptr;
  }
 private:
  VkDeviceMemory* memory_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYHANDLE_H_
