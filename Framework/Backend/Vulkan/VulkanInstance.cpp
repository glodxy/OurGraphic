//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanInstance.h"
#include "vulkan/vulkan.h"
#include "Utils/OGLogging.h"
namespace {
const char* APP_NAME = "Test Vulkan";
}

void our_graph::VulkanInstance::CreateInstance() {
  VkApplicationInfo appinfo ={};
  appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appinfo.pNext = nullptr;
  appinfo.pApplicationName = APP_NAME;
  appinfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appinfo.pEngineName = "None";
  appinfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  appinfo.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.pApplicationInfo = &appinfo;

  VkResult res = vkCreateInstance(&create_info, nullptr, &vk_instance_);
  if(res != VK_SUCCESS) {
    LOG_ERROR("CreateInstance", "create vulkan failed! res:{}", res);
    return;
  }
}

void our_graph::VulkanInstance::DestroyInstance() {
  vkDestroyInstance(vk_instance_, nullptr);
}