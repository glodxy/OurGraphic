//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANINSTANCE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANINSTANCE_H_
#include <vector>
#include "Framework/Backend/include/IRenderInstance.h"
#include "../include_internal/IPlatform.h"
#include "VulkanDef.h"
namespace our_graph {
class VulkanInstance : public IRenderInstance {
 public:
  VulkanInstance(const std::vector<const char*>& ext_names);

  void CreateInstance() override;

  void DestroyInstance() override;

  VkInstance GetInstance() {
    return vk_instance_;
  }
 private:
  bool VerifyExtLayers(std::vector<const char *> &layers);

  // 创建和销魂Debug记录的函数指针
  PFN_vkCreateDebugReportCallbackEXT create_debug_report_callback_ext_;
  PFN_vkDestroyDebugReportCallbackEXT destroy_debug_report_callback_ext_;

  VkDebugReportCallbackEXT debug_report_callback_ext_;
  // 输出debug信息的函数，此处将其转发至本地的日志模块
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  LogDebugInfo(VkFlags msg_flags, VkDebugReportObjectTypeEXT obj_type,
                    uint64_t src_obj, size_t location, int32_t msg_code,
                    const char* layer_prefix, const char* msg, void* user_data);
  // 创建信息回调
  void CreateDebugReportCallback();
  // 销毁
  void DestroyDebugReportCallback();

  VkInstance vk_instance_;

  std::vector<const char*> ext_names_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANINSTANCE_H_
