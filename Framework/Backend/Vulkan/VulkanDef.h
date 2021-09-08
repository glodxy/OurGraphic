//
// Created by Glodxy on 2021/9/8.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEF_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEF_H_

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "vulkan/vulkan.h"
#include "Utils/OGLogging.h"

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
  fpVulkan##entrypoint = (PFN_vk##entrypoint) vkGetDeviceProcAddr(instance, "vk"#entrypoint); \
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

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEF_H_
