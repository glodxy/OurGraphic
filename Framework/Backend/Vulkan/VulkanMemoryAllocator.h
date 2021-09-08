//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYALLOCATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYALLOCATOR_H_
#include <map>

#include "Framework/Backend/include/MemoryAllocator.h"
#include "VulkanDef.h"
namespace our_graph {
class VulkanMemoryAllocator : public MemoryAllocator{
 public:
  VulkanMemoryAllocator(token){}

  void Init(std::shared_ptr<IRenderDevice> device) override;
  void Clear() override;

  std::shared_ptr<MemoryHandle> AllocateGPUMemory(const std::string &name,
                                                  uint64_t size,
                                                  int memory_idx = -1) override;
  void DestroyGPUMemory(const std::string &name) override;

  bool MapGPUMemoryToLocal(const std::string &name, void **p_local) override;
  bool UnMapGPUMemory(const std::string &name) override;

  VkMemoryType GetMemoryTypeByIdx(int idx) {
    return memory_types_[idx];
  }
 private:
  // todo:将枚举更换为内部枚举，而非vulkan枚举
  uint32_t GetTypeIndex(VkMemoryPropertyFlags flag) const;
  VkDevice device_;
  /**
   * @key MemoryPropertyFlag
   * @value 支持的内存所处的index
   * */
  std::map<uint64_t, std::vector<uint32_t>> memory_flag_indices_; // 显存不同flag对应的索引列表
  std::vector<VkMemoryType> memory_types_;
};
} // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANMEMORYALLOCATOR_H_
