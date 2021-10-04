//
// Created by Glodxy on 2021/9/8.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEF_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEF_H_

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
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



class VulkanUtils {
 public:
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
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEF_H_
