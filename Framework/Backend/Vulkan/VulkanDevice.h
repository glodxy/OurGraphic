//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEVICE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEVICE_H_
#include <memory>
#include <vector>
#include "../IRenderDevice.h"
#include "vulkan/vulkan.h"
namespace our_graph {
class VulkanDevice : public IRenderDevice{
 public:
  VulkanDevice() = default;

  void CreateDevice(const std::shared_ptr<IRenderInstance> instance) override;

  void DestroyDevice() override;

  VkDevice GetDevice() {
    return device_;
  }

  uint32_t GetQueueFamilyIdx() {
    return graphic_queue_family_idx_;
  }

  VkPhysicalDeviceMemoryProperties GetMemoryProps() {
    return gpu_memory_props_;
  }
 protected:
  // 获取所有的物理设备
  bool EnumPhysicalDevices(const std::shared_ptr<IRenderInstance> instance,
                           std::vector<VkPhysicalDevice>& gpu_list);

  // 初始化队列族相关信息
  bool InitQueueFamily();

  // 创建逻辑设备
  // todo:支持多队列
  bool CreateLogicDevice();

  VkDevice device_;

  //物理设备相关
  VkPhysicalDevice* physical_device_;  // 物理设备
  VkPhysicalDeviceProperties gpu_props_;  // 物理设备属性
  VkPhysicalDeviceMemoryProperties gpu_memory_props_; // gpu显存属性

  // 队列相关, 该队列用于应用与物理设备（GPU）之间的通信
  VkQueue  queue_;
  std::vector<VkQueueFamilyProperties> queue_family_props_;
  uint32_t graphic_queue_family_idx_; // 图形队列索引号
  uint32_t queue_family_count_; // 队列族的数量

  //todo:扩展层
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDEVICE_H_
