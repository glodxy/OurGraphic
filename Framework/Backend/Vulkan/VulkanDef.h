//
// Created by Glodxy on 2021/9/8.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEF_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEF_H_

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#elif __APPLE__
#define VK_USE_PLATFORM_MACOS_MVK
#endif
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include "Utils/OGLogging.h"
#include "../include_internal/DriverEnum.h"

namespace our_graph {

/**
 * 声明函数指针
 * */
#define DECLARE_FUNC_PTR(entrypoint) PFN_vk##entrypoint fpVulkan##entrypoint

/**
 * 获取Instance的函数指针
 * */
#define INSTANCE_FUNC_PTR(instance, entrypoint) { \
  fpVulkan##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(instance, "vk"#entrypoint); \
}

#define DEVICE_FUNC_PTR(device, entrypoint) { \
  fpVulkan##entrypoint = (PFN_vk##entrypoint) vkGetDeviceProcAddr(device, "vk"#entrypoint); \
}

#define CHECK_FUNC_PTR(entrypoint) {\
  if(!fpVulkan##entrypoint) {       \
     LOG_ERROR("GetVulkanFuncPtr", "failed get "#entrypoint); \
  }\
}

#define CHECK_FUNC_PTR_RETURN(entrypoint, failedReturn) { \
  if(!fpVulkan##entrypoint) {       \
     LOG_ERROR("GetVulkanFuncPtr", "failed get "#entrypoint); \
     return failedReturn; \
  }\
}

#define INSTANCE_FUNC_AND_CHECK_RETURN(instance, entrypoint, failedValue) { \
  INSTANCE_FUNC_PTR(instance, entrypoint);                                  \
  CHECK_FUNC_PTR_RETURN(entrypoint, failedValue);\
}

#define DEVICE_FUNC_AND_CHECK_RETURN(device, entrypoint, failedValue) { \
  DEVICE_FUNC_PTR(device, entrypoint);                                  \
  CHECK_FUNC_PTR_RETURN(entrypoint, failedValue);\
}

#define CHECK_RESULT(res, tag, err_msg) { \
  if (res != VK_SUCCESS) {  \
      LOG_ERROR(tag, err_msg", code:{}", res);     \
  }                       \
}

struct VulkanLayoutTransition {
  VkImage image;
  VkImageLayout oldLayout;
  VkImageLayout newLayout;
  VkImageSubresourceRange subresources;
  VkPipelineStageFlags srcStage;
  VkAccessFlags srcAccessMask;
  VkPipelineStageFlags dstStage;
  VkAccessFlags dstAccessMask;
};

class VulkanUtils {
 public:
  /**
   * 查找符合要求的memory所属的idx
   * 一个类型可能有着多种属性
   * @param flags: 所请求的memory类型
   * @param reqs: 所请求的memory的内存属性
   * */
  static uint32_t SelectMemoryType(uint32_t flags, VkFlags reqs);

  /**
   * 查找满足条件的图像格式
   * @param candidates: 候选格式
   * @param tiling: 图像平铺格式
   * @param features: 图像用途
   * */
  static VkFormat FindSupportedFormat(
      const std::vector<VkFormat>& candidates,
      VkImageTiling tiling,
      VkFormatFeatureFlags features);

  /**
   * 将图片格式转换为普通的数据格式
   * */
  static VkFormat GetVkFormatLinear(VkFormat format);

  /**
   * 根据纹理的用处来获取相应的布局
   * */
  static VkImageLayout GetTextureLayout(TextureUsage usage);

  /**
   * 判断Rect2d是否相等
   * */
  static bool Equal(const VkRect2D& k1, const VkRect2D& k2);

  /**
   * 获取bit
   * */
   static int GetBit(uint32_t num, size_t idx);

   /**
    * 设置bit
    * */
   static void SetBit(uint32_t* num, size_t idx, bool val = true);

   static VkFormat GetVkFormat(PixelDataFormat format, PixelDataType type);

   static VkFormat GetVkFormat(TextureFormat format);

   static VkFormat GetVkFormat(ElementType type, bool is_normalized, bool is_integer);

   static bool IsDepthFormat(VkFormat format);

   static uint8_t ReduceSampleCount(uint8_t sample_count, VkSampleCountFlags mask);

   static uint32_t GetBytesPerPixel(TextureFormat format);

   static VkCompareOp GetCompareOp(SamplerCompareFunc func);

   static VkBlendFactor GetBlendFactor(BlendFunction mode);

   static VkCullModeFlags GetCullMode(CullingMode mode);

   static VkFrontFace GetFrontFace(bool inverse_front_face);

   static void TransitionImageLayout(VkCommandBuffer cmd_buffer,
                                     VulkanLayoutTransition transition);
};



}  // namespace our_graph

bool operator <(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b);

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEF_H_
