//
// Created by Glodxy on 2021/9/18.
//
#include <algorithm>
#include "VulkanDef.h"
#include "VulkanContext.h"
#include "../BackendUtils.h"

namespace our_graph {

uint32_t VulkanUtils::SelectMemoryType(uint32_t flags, VkFlags reqs) {
  VkPhysicalDeviceMemoryProperties memory_props = VulkanContext::Get().memory_properties_;
  for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; ++i) {
    // 查看目标类型中有无该类型
    if (flags & 1) {
      // 如果有，则检查设备支持该类型的memory是否满足要求
      if ((memory_props.memoryTypes[i].propertyFlags & reqs) == reqs) {
        return i;
      }
    }
    // 右移一位，表示该类型已判断
    flags >>= 1;
  }
  return uint32_t(~0ul);
}

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

VkImageLayout VulkanUtils::GetTextureLayout(TextureUsage usage) {
  if (static_cast<uint8_t>(usage) &
      static_cast<uint8_t>(TextureUsage::DEPTH_ATTACHMENT)) {
    return VK_IMAGE_LAYOUT_GENERAL;
  }

  if (static_cast<uint8_t>(usage) &
      static_cast<uint8_t>(TextureUsage::COLOR_ATTACHMENT)) {
    return VK_IMAGE_LAYOUT_GENERAL;
  }

  return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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

VkFormat VulkanUtils::GetVkFormat(TextureFormat format) {
  switch (format) {
    // 8 bits per element.
    case TextureFormat::R8:                return VK_FORMAT_R8_UNORM;
    case TextureFormat::R8_SNORM:          return VK_FORMAT_R8_SNORM;
    case TextureFormat::R8UI:              return VK_FORMAT_R8_UINT;
    case TextureFormat::R8I:               return VK_FORMAT_R8_SINT;
    case TextureFormat::STENCIL8:          return VK_FORMAT_S8_UINT;

      // 16 bits per element.
    case TextureFormat::R16F:              return VK_FORMAT_R16_SFLOAT;
    case TextureFormat::R16UI:             return VK_FORMAT_R16_UINT;
    case TextureFormat::R16I:              return VK_FORMAT_R16_SINT;
    case TextureFormat::RG8:               return VK_FORMAT_R8G8_UNORM;
    case TextureFormat::RG8_SNORM:         return VK_FORMAT_R8G8_SNORM;
    case TextureFormat::RG8UI:             return VK_FORMAT_R8G8_UINT;
    case TextureFormat::RG8I:              return VK_FORMAT_R8G8_SINT;
    case TextureFormat::RGB565:            return VK_FORMAT_R5G6B5_UNORM_PACK16;
    case TextureFormat::RGB5_A1:           return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
    case TextureFormat::RGBA4:             return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    case TextureFormat::DEPTH16:           return VK_FORMAT_D16_UNORM;

      // 24 bits per element. In practice, very few GPU vendors support these. So, we simply
      // always reshape them into 32-bit formats.
    case TextureFormat::RGB8:              return VK_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::SRGB8:             return VK_FORMAT_R8G8B8A8_SRGB;
    case TextureFormat::RGB8_SNORM:        return VK_FORMAT_R8G8B8A8_SNORM;
    case TextureFormat::RGB8UI:            return VK_FORMAT_R8G8B8A8_UINT;
    case TextureFormat::RGB8I:             return VK_FORMAT_R8G8B8A8_SINT;

    case TextureFormat::DEPTH24:
      return VK_FORMAT_UNDEFINED;

      // 32 bits per element.
    case TextureFormat::R32F:              return VK_FORMAT_R32_SFLOAT;
    case TextureFormat::R32UI:             return VK_FORMAT_R32_UINT;
    case TextureFormat::R32I:              return VK_FORMAT_R32_SINT;
    case TextureFormat::RG16F:             return VK_FORMAT_R16G16_SFLOAT;
    case TextureFormat::RG16UI:            return VK_FORMAT_R16G16_UINT;
    case TextureFormat::RG16I:             return VK_FORMAT_R16G16_SINT;
    case TextureFormat::R11F_G11F_B10F:    return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case TextureFormat::RGB9_E5:           return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    case TextureFormat::RGBA8:             return VK_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::SRGB8_A8:          return VK_FORMAT_R8G8B8A8_SRGB;
    case TextureFormat::RGBA8_SNORM:       return VK_FORMAT_R8G8B8A8_SNORM;
    case TextureFormat::RGB10_A2:          return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case TextureFormat::RGBA8UI:           return VK_FORMAT_R8G8B8A8_UINT;
    case TextureFormat::RGBA8I:            return VK_FORMAT_R8G8B8A8_SINT;
    case TextureFormat::DEPTH32F:          return VK_FORMAT_D32_SFLOAT;
    case TextureFormat::DEPTH24_STENCIL8:  return VK_FORMAT_D24_UNORM_S8_UINT;
    case TextureFormat::DEPTH32F_STENCIL8: return VK_FORMAT_D32_SFLOAT_S8_UINT;

      // 48 bits per element. In practice, very few GPU vendors support these. So, we simply
      // always reshape them into 64-bit formats.
    case TextureFormat::RGB16F:            return VK_FORMAT_R16G16B16A16_SFLOAT;
    case TextureFormat::RGB16UI:           return VK_FORMAT_R16G16B16A16_UINT;
    case TextureFormat::RGB16I:            return VK_FORMAT_R16G16B16A16_SINT;

      // 64 bits per element.
    case TextureFormat::RG32F:             return VK_FORMAT_R32G32_SFLOAT;
    case TextureFormat::RG32UI:            return VK_FORMAT_R32G32_UINT;
    case TextureFormat::RG32I:             return VK_FORMAT_R32G32_SINT;
    case TextureFormat::RGBA16F:           return VK_FORMAT_R16G16B16A16_SFLOAT;
    case TextureFormat::RGBA16UI:          return VK_FORMAT_R16G16B16A16_UINT;
    case TextureFormat::RGBA16I:           return VK_FORMAT_R16G16B16A16_SINT;

      // 96-bits per element.
    case TextureFormat::RGB32F:            return VK_FORMAT_R32G32B32_SFLOAT;
    case TextureFormat::RGB32UI:           return VK_FORMAT_R32G32B32_UINT;
    case TextureFormat::RGB32I:            return VK_FORMAT_R32G32B32_SINT;

      // 128-bits per element.
    case TextureFormat::RGBA32F:           return VK_FORMAT_R32G32B32A32_SFLOAT;
    case TextureFormat::RGBA32UI:          return VK_FORMAT_R32G32B32A32_UINT;
    case TextureFormat::RGBA32I:           return VK_FORMAT_R32G32B32A32_SINT;

      // Compressed textures.
    case TextureFormat::DXT1_RGB:          return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case TextureFormat::DXT1_SRGB:         return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    case TextureFormat::DXT1_RGBA:         return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case TextureFormat::DXT1_SRGBA:        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case TextureFormat::DXT3_RGBA:         return VK_FORMAT_BC2_UNORM_BLOCK;
    case TextureFormat::DXT3_SRGBA:        return VK_FORMAT_BC2_SRGB_BLOCK;
    case TextureFormat::DXT5_RGBA:         return VK_FORMAT_BC3_UNORM_BLOCK;
    case TextureFormat::DXT5_SRGBA:        return VK_FORMAT_BC3_SRGB_BLOCK;

    case TextureFormat::RGBA_ASTC_4x4:     return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_5x4:     return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_5x5:     return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_6x5:     return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_6x6:     return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_8x5:     return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_8x6:     return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_8x8:     return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_10x5:    return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_10x6:    return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_10x8:    return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_10x10:   return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_12x10:   return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
    case TextureFormat::RGBA_ASTC_12x12:   return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;

    case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:   return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:   return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:   return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:   return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:   return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:   return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:   return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:   return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:  return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:  return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:  return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_10x10: return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_12x10: return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
    case TextureFormat::SRGB8_ALPHA8_ASTC_12x12: return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

    case TextureFormat::ETC2_RGB8:         return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case TextureFormat::ETC2_SRGB8:        return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
    case TextureFormat::ETC2_RGB8_A1:      return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
    case TextureFormat::ETC2_SRGB8_A1:     return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;

    case TextureFormat::ETC2_EAC_RGBA8:    return VK_FORMAT_UNDEFINED;
    case TextureFormat::ETC2_EAC_SRGBA8:   return VK_FORMAT_UNDEFINED;

    case TextureFormat::EAC_R11:           return VK_FORMAT_EAC_R11_UNORM_BLOCK;
    case TextureFormat::EAC_R11_SIGNED:    return VK_FORMAT_EAC_R11_SNORM_BLOCK;
    case TextureFormat::EAC_RG11:          return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
    case TextureFormat::EAC_RG11_SIGNED:   return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

    default:
      return VK_FORMAT_UNDEFINED;
  }
}

bool VulkanUtils::IsDepthFormat(VkFormat format) {
  switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return true;
    default:
      return false;
  }
}


static uint32_t MostSignificantBit(uint32_t x) { return 1ul << (31ul - (x)); }

uint8_t VulkanUtils::ReduceSampleCount(uint8_t sample_count, VkSampleCountFlags mask) {
  if (sample_count & mask) {
    return sample_count;
  }
  uint32_t tmp = ((sample_count -1) & mask);
  // 取tmp的最高位
  // todo:优化
  uint32_t res= (1u << 31);
  // 找到第一个不为0的位为止
  while((res & tmp) == 0 && res > 1) {
    res >>= 1;
  }
  // 采样数最低为1
  return res;
}

VkFormat VulkanUtils::GetVkFormatLinear(VkFormat format) {
  switch (format) {
    case VK_FORMAT_R8_SRGB: return VK_FORMAT_R8_UNORM;
    case VK_FORMAT_R8G8_SRGB: return VK_FORMAT_R8G8_UNORM;
    case VK_FORMAT_R8G8B8_SRGB: return VK_FORMAT_R8G8B8_UNORM;
    case VK_FORMAT_B8G8R8_SRGB: return VK_FORMAT_B8G8R8_UNORM;
    case VK_FORMAT_R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_UNORM;
    case VK_FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_UNORM;
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case VK_FORMAT_BC2_SRGB_BLOCK: return VK_FORMAT_BC2_UNORM_BLOCK;
    case VK_FORMAT_BC3_SRGB_BLOCK: return VK_FORMAT_BC3_UNORM_BLOCK;
    case VK_FORMAT_BC7_SRGB_BLOCK: return VK_FORMAT_BC7_UNORM_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG: return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG: return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
    default: return format;
  }
  return format;
}

uint32_t VulkanUtils::GetBytesPerPixel(TextureFormat format) {
  return (uint32_t)BackendUtil::GetFormatSize(format);
}

}  // namespace our_graph

bool operator<(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b) {
  if (a.aspectMask < b.aspectMask) return true;
  if (a.aspectMask > b.aspectMask) return false;
  if (a.baseMipLevel < b.baseMipLevel) return true;
  if (a.baseMipLevel > b.baseMipLevel) return false;
  if (a.levelCount < b.levelCount) return true;
  if (a.levelCount > b.levelCount) return false;
  if (a.baseArrayLayer < b.baseArrayLayer) return true;
  if (a.baseArrayLayer > b.baseArrayLayer) return false;
  if (a.layerCount < b.layerCount) return true;
  if (a.layerCount > b.layerCount) return false;
  return false;
}