//
// Created by Glodxy on 2021/9/18.
//

#include "VulkanDef.h"
#include "VulkanContext.h"

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

// copy from filament
VkFormat VulkanUtils::GetVkFormat(PixelDataFormat format, PixelDataType type) {
  if (type == PixelDataType::USHORT_565) return VK_FORMAT_R5G6B5_UNORM_PACK16;
  if (type == PixelDataType::UINT_2_10_10_10_REV) return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
  if (type == PixelDataType::UINT_10F_11F_11F_REV) return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

#define CONVERT(FORMAT, TYPE, VK) \
    if (PixelDataFormat::FORMAT == format && PixelDataType::TYPE == type)  return VK_FORMAT_ ## VK;

  CONVERT(R, UBYTE, R8_UNORM);
  CONVERT(R, BYTE, R8_SNORM);
  CONVERT(R_INTEGER, UBYTE, R8_UINT);
  CONVERT(R_INTEGER, BYTE, R8_SINT);
  CONVERT(RG, UBYTE, R8G8_UNORM);
  CONVERT(RG, BYTE, R8G8_SNORM);
  CONVERT(RG_INTEGER, UBYTE, R8G8_UINT);
  CONVERT(RG_INTEGER, BYTE, R8G8_SINT);
  CONVERT(RGBA, UBYTE, R8G8B8A8_UNORM);
  CONVERT(RGBA, BYTE, R8G8B8A8_SNORM);
  CONVERT(RGBA_INTEGER, UBYTE, R8G8B8A8_UINT);
  CONVERT(RGBA_INTEGER, BYTE, R8G8B8A8_SINT);
  CONVERT(R_INTEGER, USHORT, R16_UINT);
  CONVERT(R_INTEGER, SHORT, R16_SINT);
  CONVERT(R, HALF, R16_SFLOAT);
  CONVERT(RG_INTEGER, USHORT, R16G16_UINT);
  CONVERT(RG_INTEGER, SHORT, R16G16_SINT);
  CONVERT(RG, HALF, R16G16_SFLOAT);
  CONVERT(RGBA_INTEGER, USHORT, R16G16B16A16_UINT);
  CONVERT(RGBA_INTEGER, SHORT, R16G16B16A16_SINT);
  CONVERT(RGBA, HALF, R16G16B16A16_SFLOAT);
  CONVERT(R_INTEGER, UINT, R32_UINT);
  CONVERT(R_INTEGER, INT, R32_SINT);
  CONVERT(R, FLOAT, R32_SFLOAT);
  CONVERT(RG_INTEGER, UINT, R32G32_UINT);
  CONVERT(RG_INTEGER, INT, R32G32_SINT);
  CONVERT(RG, FLOAT, R32G32_SFLOAT);
  CONVERT(RGBA_INTEGER, UINT, R32G32B32A32_UINT);
  CONVERT(RGBA_INTEGER, INT, R32G32B32A32_SINT);
  CONVERT(RGBA, FLOAT, R32G32B32A32_SFLOAT);
#undef CONVERT

  return VK_FORMAT_UNDEFINED;
}
}  // namespace our_graph