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
                               VkImageCreateInfo create_info);

  VulkanTextureBuffer(const std::string& name,
                               VkDevice device,
                               VkImage image);

  void * GetInstance() override;

  ~VulkanTextureBuffer()override;
 private:
  void Create() override;
 private:
  bool CreateImage();
  bool AllocateMemory();

  VkDevice device_;
  VkImageCreateInfo create_info_;
  VkImage image_;
  std::string name_;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTUREBUFFER_H_
