//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYALLOCATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYALLOCATOR_H_
#include <map>

#include "../IMemoryAllocator.h"
#include "vulkan/vulkan.h"
namespace our_graph {
class VulkanMemoryAllocator : public IMemoryAllocator{
 public:
  VulkanMemoryAllocator(token){}

  void Init(std::shared_ptr<IRenderDevice> device) override;
  void Clear() override;

  std::shared_ptr<IMemoryHandle> AllocateGPUMemory(const std::string &name, uint64_t size) override;
  void DestroyGPUMemory(const std::string &name) override;

  bool MapGPUMemoryToLocal(const std::string &name, void **p_local) override;
  bool UnMapGPUMemory(const std::string &name) override;
 private:
  // todo:将枚举更换为内部枚举，而非vulkan枚举
  uint32_t GetTypeIndex(VkMemoryPropertyFlags flag) const;
  VkDevice device_;
  std::map<uint64_t, std::vector<uint32_t>> memory_flag_indices_; // 显存不同flag对应的索引列表
};
} // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYALLOCATOR_H_
