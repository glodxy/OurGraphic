//
// Created by Glodxy on 2021/8/29.
//

#include "VulkanDevice.h"

#include "Utils/OGLogging.h"
#include "VulkanInstance.h"

namespace {
const std::vector<const char*> DEVICE_EXTENSION_NAMES = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
}
void our_graph::VulkanDevice::CreateDevice(const std::shared_ptr<IRenderInstance> instance) {
  std::vector<VkPhysicalDevice> gpu_list;
  //获取所有的gpu
  if (!EnumPhysicalDevices(instance, gpu_list)) {
    LOG_ERROR("CreateDevice", "vulkan enum gpu list failed!");
    return;
  }

  if (gpu_list.empty()) {
    LOG_ERROR("CreateDevice", "no gpu available");
    return;
  }

  // 此处默认选择第一个gpu
  physical_device_ = gpu_list[0];
  // 获取物理设备的属性
  vkGetPhysicalDeviceProperties(physical_device_, &gpu_props_);
  // 获取物理设备的内存属性
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &gpu_memory_props_);

  // 输出gpu信息
  LOG_INFO("CreateDevice", "GPU info id:{} >>>>>>>>>>>>>>>>>>>>", gpu_props_.deviceID);
  LOG_INFO("CreateDevice", "GPU Name:{}, \n"
                           "GPU api_version:{} \n"
                           "GPU driver_version:{}",
                           gpu_props_.deviceName,
                           gpu_props_.apiVersion,
                           gpu_props_.driverVersion);
  LOG_INFO("CreateDevice", "GPU info <<<<<<<<<<<<<<<<<<<<<<<<<<");
  //初始化队列相关
  if (!InitQueueFamily()) {
    LOG_ERROR("CreateDevice", "vulkan InitQueueFamily failed!");
    return;
  }

  // 创建逻辑设备
  if (!CreateLogicDevice()) {
    LOG_ERROR("CreateDevice", "vulkan CreateLogicDevice failed!");
    return;
  }

  // 将逻辑设备的queue取出
  // 目前仅使用了一个队列
  vkGetDeviceQueue(device_, graphic_queue_family_idx_, 0, &queue_);

  // 设置context
  VulkanContext::Get().device_ = &device_;
  VulkanContext::Get().physical_device_ = &physical_device_;
  VulkanContext::Get().graphic_queue_family_idx_ = graphic_queue_family_idx_;
  VulkanContext::Get().graphic_queue_ = &queue_;
}

void our_graph::VulkanDevice::DestroyDevice() {
  physical_device_ = nullptr;
  vkDestroyDevice(device_, nullptr);
}

bool our_graph::VulkanDevice::InitQueueFamily() {
  // 查询队列族的数量
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device_,
                                           &queue_family_count_,
                                           nullptr);
  queue_family_props_.resize(queue_family_count_);
  // 将属性写入vector
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device_,
                                           &queue_family_count_,
                                           queue_family_props_.data());

  bool has_graphic_queue = false;
  // 获取图形队列
  for (int i = 0; i < queue_family_count_; ++i) {
    // 此处查找第一个满足的队列族
    if (queue_family_props_[i].queueFlags &
        VK_QUEUE_GRAPHICS_BIT) {
      //找到图形队列
      has_graphic_queue = true;
      graphic_queue_family_idx_ = i;
      break;
    }
  }

  if (!has_graphic_queue) {
    LOG_ERROR("InitQueueFamily", "vulkan graphic queue not found!");
    return false;
  }
  return true;
}

bool our_graph::VulkanDevice::CreateLogicDevice() {
  // 要创建的各个队列的优先级，目前仅有一个队列
  float queue_priority[1] = {0.0f};

  VkDeviceQueueCreateInfo queue_info ={};
  queue_info.queueFamilyIndex = graphic_queue_family_idx_;
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = nullptr;
  queue_info.queueCount = 1; // 目前仅使用单队列
  queue_info.pQueuePriorities = queue_priority;


  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = nullptr;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info; //目前仅1个队列族
  device_info.enabledLayerCount = 0;
  //todo:扩展层
  device_info.enabledExtensionCount = DEVICE_EXTENSION_NAMES.size();
  device_info.ppEnabledExtensionNames = DEVICE_EXTENSION_NAMES.data();

  // 创建逻辑设备
  VkResult res = vkCreateDevice(physical_device_, &device_info, nullptr, &device_);
  if (res != VK_SUCCESS) {
    LOG_ERROR("CreateLogicDevice", "vulkan create logic device failed! res:{}", res);
    return false;
  }

  return true;
}

bool our_graph::VulkanDevice::EnumPhysicalDevices(const std::shared_ptr<IRenderInstance> instance,
                                                  std::vector<VkPhysicalDevice> &gpu_list) {
  uint32_t gpu_count;
  VulkanInstance* vk_ins = dynamic_cast<VulkanInstance*>(instance.get());
  VkInstance real_instance = vk_ins->GetInstance();
  // 获取gpu数量
  vkEnumeratePhysicalDevices(real_instance, &gpu_count, nullptr);
  gpu_list.resize(gpu_count);

  VkResult res = vkEnumeratePhysicalDevices(real_instance, &gpu_count, gpu_list.data());
  if (res != VK_SUCCESS) {
    LOG_ERROR("EnumPhysicalDevices", "vulkan get devices failed! res:{}", res);
    return false;
  }

  return true;
}