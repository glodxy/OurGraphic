//
// Created by Glodxy on 2021/9/1.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTURE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTURE_H_
#include "Framework/Backend/include/ITexture.h"
#include "VulkanDef.h"
namespace our_graph {
class VulkanTexture : public ITexture {
 public:
  explicit VulkanTexture(const std::string& name,
                VkDevice device,
                VkImage image,
                VkImageView view,
                bool need_buffer = true);

  /**
   * 根据创建信息来创建纹理
   * */
  explicit VulkanTexture(const std::string& name,
                         VkDevice device,
                         VkImageCreateInfo image_create_info,
                         VkImageViewCreateInfo view_create_info,
                         uint64_t memory_flag_bits = 0);

  ~VulkanTexture() override;
  std::shared_ptr<IDescriptor> GetView() override;
  std::shared_ptr<IBuffer> GetBuffer() override;
 private:
  void Create() override;
  void Destroy() override;


 protected:

  VkImageCreateInfo GetImageCreateInfo() const;
  VkImageViewCreateInfo GetImageViewCreateInfo() const;

 private:
  VkDevice device_;
  std::string name_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTURE_H_
