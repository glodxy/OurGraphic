//
// Created by Glodxy on 2021/9/19.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBUFFER_H_
#include <memory>
#include "../include_internal/IBuffer.h"
#include "VulkanDef.h"

namespace our_graph {
class VulkanBuffer : public IBuffer{
 public:
  /**
   * 以创建的形式生成buffer
   * 该Buffer的memory以及实例自动创建
   * */
  explicit VulkanBuffer(const std::string& name,
                               VkDevice device,
                               VkBufferCreateInfo create_info,
                               uint64_t memory_flag_bits = 0);

  /**
   * 外部创建实例，该Buffer仅进行管理
   * @param auto_control:是否由内部管理physical_buffer
   * */
  explicit VulkanBuffer(const std::string& name,
                        VkDevice device,
                        VkBuffer buffer,
                        bool auto_control = true);

  void * GetInstance() override;

  void* GetLocalMemory();

  /**
   * 写入数据
   * @param src：源数据起始位置
   * @param size：要写入的大小
   * */
  bool WriteData(void* src, int size);

  ~VulkanBuffer()override;

 protected:
  void Create() override;


  /**
   * CreateBuffer->AllocateMemory->
   * -> BindMemory
   * */
  bool CreateBuffer();
  bool AllocateMemory();

  /**
   * MapMemory->Write->UnMapMemory
   * */
  bool MapMemory();
  bool UnMapMemory();

 protected:
  uint64_t flag_bits_ {0}; // 要使用的显存类型
  VkDevice device_;
  VkBuffer buffer_;
  VkBufferCreateInfo buffer_create_info_;
  std::string name_;
  // todo:迁移至统一的内存池（stage pool）
  uint8_t* local_memory_{nullptr};
  /**
   * 是否是正在映射的状态
   * 只有处于映射状态才能写入数据
   * */
  bool is_mapping_ {false};
  bool auto_control_ {true}; // 是否需要内部管理memory
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBUFFER_H_
