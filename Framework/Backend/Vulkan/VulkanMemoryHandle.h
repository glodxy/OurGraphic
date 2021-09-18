//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYHANDLE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYHANDLE_H_
#include "Framework/Backend/include/MemoryHandle.h"
#include "VulkanDef.h"
namespace our_graph {
/**
 * vulkan的显存句柄
 * */
class VulkanMemoryHandle : public MemoryHandle{
 public:
  explicit VulkanMemoryHandle(VkDeviceMemory* memory,
                              const std::string& name,
                              uint64_t size);
  void * GetMemory() override {
    return memory_;
  }

  ~VulkanMemoryHandle() override{
    memory_ = VK_NULL_HANDLE;
  }
 private:
  VkDeviceMemory* memory_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYHANDLE_H_
