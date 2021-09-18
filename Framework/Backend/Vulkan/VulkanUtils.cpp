//
// Created by Glodxy on 2021/9/18.
//

#include "VulkanDef.h"

namespace our_graph {

VkFormat VulkanUtils::FindSupportedFormat(
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features) {
  for (VkFormat format : candidates)
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(*(VulkanContext::Get().physical_device_), format, &props);

    if ((tiling == VK_IMAGE_TILING_LINEAR) &&
    ((props.linearTilingFeatures & features) == features))
    {
      return format;
    }
    else if ((tiling == VK_IMAGE_TILING_OPTIMAL) &&
    ((props.optimalTilingFeatures & features) == features))
    {
      return format;
    }
  }
  return VK_FORMAT_UNDEFINED;
}
}  // namespace our_graph