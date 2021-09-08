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
  void Create(std::shared_ptr<IRenderDevice> args) override;
  void Destroy() override;

  std::shared_ptr<ITextureView> GetView() override;
 protected:

  VkImageCreateInfo GetImageCreateInfo() const;
  VkImageViewCreateInfo GetImageViewCreateInfo() const;

 private:
  VkDevice device_;
  std::string name_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTURE_H_
