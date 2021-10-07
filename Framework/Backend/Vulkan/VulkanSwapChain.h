//
// Created by Glodxy on 2021/9/8.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSWAPCHAIN_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSWAPCHAIN_H_
#include <vector>
#include "../include_internal/IPlatform.h"
#include "VulkanTexture.h"
#include "VulkanContext.h"
#include "VulkanDef.h"
namespace our_graph {
class VulkanSwapChain {
  using Vec2i32 = VkExtent2D;
 public:
  VulkanSwapChain(VkDevice device,
                  VkInstance instance,
                  std::shared_ptr<IPlatform> platform,
                  void* window_handle);

  void Destroy();

  int GetRenderTargetCnt() const;


 private:
  void Create(void* handle);
  // 设置扩展信息
  bool CreateSwapChainExt();
  // 创建表面
  bool CreateSurface(void* handle);
  // 创建交换链
  bool CreateSwapChain();

  // 从交换链中取出相应的image
  bool BuildTexture();

  // 创建深度图
  bool CreateDepthImage(VkFormat format, Vec2i32 size);

  // 查找可用于显示的队列
  bool FindPresentationQueue();

  inline std::string GetName(int idx) const {
    return ("RenderTarget" + std::to_string(idx));
  }
 protected:
  VkSurfaceCapabilitiesKHR surface_param_ = {}; // 窗口表面属性
  std::vector<VkPresentModeKHR> present_modes_; // 窗口展示模式
  std::vector<VkSurfaceFormatKHR> surface_formats_; // 表面格式

  Vec2i32 swapchain_image_size_; // 交换链的缓冲大小(即每张图片的尺寸大小)

  uint32_t swapchain_image_cnt_available_; // 交换链可用的缓冲数量
  VkSurfaceTransformFlagBitsKHR transform_flag_;

  // 交换链中所有的图像
  std::vector<VulkanAttachment> swapchain_images_;
  // 深度图像
  VulkanAttachment depth_;
  VkSemaphore image_available = {}; // 请求下一张image成功时会设置该信号量
  VkImageLayout depth_layout_; // 深度图的布局

  uint32_t graphic_queue_family_idx_; // 用于图形显示的队列idx
 private:
  VkSurfaceKHR surface_ = {}; // 表面对象
  VkSurfaceFormatKHR surface_format_ = {}; // 表面对象的格式(RGBA)
  VkSwapchainKHR swapchain_;

  uint32_t current_idx_; // 当前使用的索引

  VkDevice device_;
  VkPhysicalDevice physical_device_;
  VkInstance instance_;

  // 窗口相关
  void*  window_handle_;

  VkQueue present_queue_; //用于交换链的队列

  std::shared_ptr<IPlatform> platform_;
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
