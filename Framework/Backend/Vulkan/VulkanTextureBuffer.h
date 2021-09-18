//
// Created by Glodxy on 2021/9/6.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTUREBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTUREBUFFER_H_
#include <memory>
#include "../include_internal/IBuffer.h"
#include "VulkanDef.h"
namespace our_graph {
class VulkanTextureBuffer : public IBuffer{
 public:
  explicit VulkanTextureBuffer(const std::string& name,
                               VkDevice device,
                               VkImageCreateInfo create_info,
                               uint64_t memory_flag_bits = 0);

  VulkanTextureBuffer(const std::string& name,
                               VkDevice device,
                               VkImage image,
                               bool need_buffer);

  void * GetInstance() override;

  ~VulkanTextureBuffer()override;
 private:
  void Create() override;
 private:
  bool CreateImage();
  bool AllocateMemory();

  uint64_t flag_bits_ {0}; // 要使用的显存类型
  VkDevice device_;
  VkImageCreateInfo create_info_;
  VkImage image_;
  std::string name_;
  bool has_buffer_ {true}; // 是否存在管理的显存，没有代表为vulkan自动管理
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTUREBUFFER_H_
