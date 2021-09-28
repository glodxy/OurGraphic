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

bool VulkanUtils::Equal(const VkRect2D &k1, const VkRect2D &k2) {
  return k1.extent.width == k2.extent.width && k1.extent.height == k2.extent.height
        && k1.offset.x == k2.offset.x && k1.offset.y == k2.offset.y;
}

int VulkanUtils::GetBit(uint32_t num, size_t idx) {
  return (num & (1 << idx)) == 0 ? 0 : 1;
}

void VulkanUtils::SetBit(uint32_t *num, size_t idx, bool val) {
  if (val) {
    (*num) = ((*num) | (1 << idx));
    return;
  }
  (*num) = ((*num) & (~(1 << idx)));
  (*num) = ((*num) | (val << idx));
}
}  // namespace our_graph