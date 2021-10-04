//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanInstance.h"
#include "VulkanDef.h"
#include "Utils/OGLogging.h"
#include "VulkanContext.h"

namespace {
const char* APP_NAME = "Test Vulkan";
const std::vector<const char*> INSTANCE_EXT_NAMES = {
    "VK_KHR_surface",
    "VK_KHR_get_physical_device_properties2",
    "VK_EXT_debug_utils",
    "VK_EXT_debug_report",
#ifdef _WIN32
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
};

const std::vector<const char*> LAYER_NAMES = {
    "VK_LAYER_KHRONOS_validation",
};
}

our_graph::VulkanInstance::VulkanInstance(const std::vector<const char *> &ext_names) {
  ext_names_ = ext_names;
}


void our_graph::VulkanInstance::CreateInstance() {
  // 创建instance实例
  VkApplicationInfo appinfo ={};
  appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appinfo.pNext = nullptr;
  appinfo.pApplicationName = APP_NAME;
  appinfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appinfo.pEngineName = "None";
  appinfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
#if _WIN32
  appinfo.apiVersion = VK_API_VERSION_1_2;
#elif __APPLE__
  appinfo.apiVersion = VK_API_VERSION_1_2;
#endif

  std::vector<const char*> layers = LAYER_NAMES;

  std::vector<const char*> instance_extensions(INSTANCE_EXT_NAMES.begin(), INSTANCE_EXT_NAMES.end());
  instance_extensions.insert(instance_extensions.end(), ext_names_.begin(), ext_names_.end());

  VerifyExtLayers(layers);



  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.pApplicationInfo = &appinfo;
  create_info.enabledLayerCount = layers.size();
  create_info.ppEnabledLayerNames = layers.data();
  create_info.enabledExtensionCount = instance_extensions.size();
  create_info.ppEnabledExtensionNames = instance_extensions.data();

  VkResult res = vkCreateInstance(&create_info, nullptr, &vk_instance_);
  if(res != VK_SUCCESS) {
    LOG_ERROR("CreateInstance", "create vulkan failed! res:{}", res);
    return;
  }
  CreateDebugReportCallback();
}

void our_graph::VulkanInstance::DestroyInstance() {
  DestroyDebugReportCallback();
  vkDestroyInstance(vk_instance_, nullptr);
}

bool our_graph::VulkanInstance::VerifyExtLayers(std::vector<const char *> &layers) {
  uint32_t check_count = layers.size();
  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> supported_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, supported_layers.data());
  std::vector<const char*> unsupported_layers;
  LOG_INFO("VulkanExtLayers", "supported size:{}", layer_count);
  for (uint32_t i = 0; i < check_count; ++i) {
    bool is_supported = false;
    for (uint32_t j = 0; j < layer_count; ++j) {
      if (!strcmp(layers[i], supported_layers[j].layerName)) {
        is_supported = true;
        break;
      }
    }
    if (!is_supported) {
      LOG_WARN("VulkanExtLayers", "layer[{}] not supported!",
               layers[i]);
      unsupported_layers.push_back(layers[i]);
    }
    else {
      LOG_INFO("VulkanExtLayers", "layer[{}] supported!",
               layers[i]);
    }
  }

  for (auto i : unsupported_layers) {
    auto iter = std::find(layers.begin(), layers.end(), i);
    if (iter != layers.end()) {
      layers.erase(iter);
    }
  }
  return true;
}

void our_graph::VulkanInstance::CreateDebugReportCallback() {
  create_debug_report_callback_ext_ =
      (PFN_vkCreateDebugReportCallbackEXT)
          vkGetInstanceProcAddr(vk_instance_, "vkCreateDebugReportCallbackEXT");
  if (!create_debug_report_callback_ext_) {
    LOG_ERROR("VulkanExtLayers", "Failed to locate vkCreateDebugReportCallbackEXT!");
  }
  destroy_debug_report_callback_ext_ =
      (PFN_vkDestroyDebugReportCallbackEXT)
          vkGetInstanceProcAddr(vk_instance_, "vkDestroyDebugReportCallbackEXT");
  if (!destroy_debug_report_callback_ext_) {
    LOG_ERROR("VulkanExtLayers", "Failed to locate vkDestroyDebugReportCallbackEXT");
  }

  VkDebugReportCallbackCreateInfoEXT create_info;
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
  create_info.pfnCallback = LogDebugInfo;
  create_info.pNext = nullptr;
  create_info.pUserData = nullptr;
  create_info.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                      VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                      VK_DEBUG_REPORT_ERROR_BIT_EXT |
                      VK_DEBUG_REPORT_DEBUG_BIT_EXT;
  if (create_debug_report_callback_ext_) {
    create_debug_report_callback_ext_(vk_instance_, &create_info, nullptr, &debug_report_callback_ext_);
  }
}

void our_graph::VulkanInstance::DestroyDebugReportCallback() {
  if (destroy_debug_report_callback_ext_) {
    destroy_debug_report_callback_ext_(vk_instance_, debug_report_callback_ext_, nullptr);
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL our_graph::VulkanInstance::LogDebugInfo(VkFlags msg_flags,
                                                 VkDebugReportObjectTypeEXT obj_type,
                                                 uint64_t src_obj,
                                                 size_t location,
                                                 int32_t msg_code,
                                                 const char *layer_prefix,
                                                 const char *msg,
                                                 void *user_data) {
  if (msg_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    SIMPLE_LOG(2, "[VK_DEBUG_REPORT] ERROR: [{}] Code {} : {}",
               layer_prefix, msg_code, msg);
  } else if (msg_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    SIMPLE_LOG(1, "[VK_DEBUG_REPORT] WARNING: [{}] Code {} : {}",
               layer_prefix, msg_code, msg);
  } else if (msg_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    SIMPLE_LOG(1, "[VK_DEBUG_REPORT] INFORMATION: [{}] Code {} : {}",
               layer_prefix, msg_code, msg);
  } else if (msg_flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    SIMPLE_LOG(1, "[VK_DEBUG_REPORT] PERFORMANCE: [{}] Code {} : {}",
               layer_prefix, msg_code, msg);
  } else if (msg_flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    SIMPLE_LOG(0, "[VK_DEBUG_REPORT] DEBUG: [{}] Code {} : {}",
               layer_prefix, msg_code, msg);
  } else {
    return VK_FALSE;
  }

  return VK_SUCCESS;
}