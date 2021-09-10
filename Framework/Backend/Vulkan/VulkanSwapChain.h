//
// Created by Glodxy on 2021/9/8.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSWAPCHAIN_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSWAPCHAIN_H_
#include <vector>
#include "../include/ISwapchain.h"
#include "VulkanTextureBuffer.h"
#include "VulkanDef.h"
namespace our_graph {
class VulkanSwapChain : public ISwapChain{
  using Vec2i32 = VkExtent2D;
 public:
  explicit VulkanSwapChain(VkDevice device, VkInstance instance);
  void Create(WindowInstance ins, WindowHandle handle) override;
  void Destroy() override;

  int GetRenderTargetCnt() const override;
  std::shared_ptr<ITexture> GetRenderTarget(int idx) override;

 private:
  // 设置扩展信息
  bool CreateSwapChainExt();
  // 创建表面
  bool CreateSurface(WindowInstance ins, WindowHandle handle);
 protected:
  VkSurfaceCapabilitiesKHR surface_param_; // 窗口表面属性
  std::vector<VkPresentModeKHR> present_modes_; // 窗口展示模式
  std::vector<VkSurfaceFormatKHR> surface_formats_; // 表面格式

  Vec2i32 swapchain_image_size_; // 交换链的缓冲大小

  uint32_t swapchain_buffer_cnt_available_; // 交换链可用的缓冲数量
  VkSurfaceTransformFlagBitsKHR transform_flag_;

  std::vector<std::shared_ptr<VulkanTextureBuffer>> swapchain_buffers_;
 private:
  VkSurfaceKHR surface_; // 表面对象
  uint32_t swapchain_buffer_cnt_; // 交换链当前使用的数量
  VkSwapchainKHR swapchain_;

  uint32_t current_idx_; // 当前使用的索引
  VkFormat format_; // buffer格式

  VkDevice device_;
  VkInstance instance_;


 private:
  DECLARE_FUNC_PTR(GetPhysicalDeviceSurfaceSupportKHR);
  DECLARE_FUNC_PTR(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  DECLARE_FUNC_PTR(GetPhysicalDeviceSurfaceFormatsKHR);
  DECLARE_FUNC_PTR(GetPhysicalDeviceSurfacePresentModesKHR);
  DECLARE_FUNC_PTR(DestroySurfaceKHR);

  DECLARE_FUNC_PTR(CreateSwapchainKHR);
  DECLARE_FUNC_PTR(DestroySwapchainKHR);
  DECLARE_FUNC_PTR(GetSwapchainImagesKHR);
  DECLARE_FUNC_PTR(AcquireNextImageKHR);
  DECLARE_FUNC_PTR(QueuePresentKHR);
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSWAPCHAIN_H_
