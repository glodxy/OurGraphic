//
// Created by Glodxy on 2021/10/10.
//

#include "VulkanSamplerCache.h"
#include "VulkanContext.h"

namespace our_graph {

constexpr inline VkSamplerAddressMode GetWrapMode(SamplerWrapMode mode) noexcept {
  switch (mode) {
    case SamplerWrapMode::REPEAT:
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case SamplerWrapMode::CLAMP_TO_EDGE:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case SamplerWrapMode::MIRRORED_REPEAT:
      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  }
}

constexpr inline VkFilter GetFilter(SamplerMinFilter filter) noexcept {
  switch (filter) {
    case SamplerMinFilter::NEAREST:
      return VK_FILTER_NEAREST;
    case SamplerMinFilter::LINEAR:
      return VK_FILTER_LINEAR;
    case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
      return VK_FILTER_NEAREST;
    case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
      return VK_FILTER_LINEAR;
    case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
      return VK_FILTER_NEAREST;
    case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
      return VK_FILTER_LINEAR;
  }
}

constexpr inline VkFilter GetFilter(SamplerMagFilter filter) noexcept {
  switch (filter) {
    case SamplerMagFilter::NEAREST:
      return VK_FILTER_NEAREST;
    case SamplerMagFilter::LINEAR:
      return VK_FILTER_LINEAR;
  }
}

constexpr inline VkSamplerMipmapMode GetMipmapMode(SamplerMinFilter filter) noexcept {
  switch (filter) {
    case SamplerMinFilter::NEAREST:
      return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case SamplerMinFilter::LINEAR:
      return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
      return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
      return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
      return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
      return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
}

constexpr inline float GetMaxLod(SamplerMinFilter filter) noexcept {
  switch (filter) {
    case SamplerMinFilter::NEAREST:
    case SamplerMinFilter::LINEAR:
      // The Vulkan spec recommends a max LOD of 0.25 to "disable" mipmapping.
      // See "Mapping of OpenGL to Vulkan filter modes" in the VK Spec.
      return 0.25f;
    case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
    case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
    case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
    case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
      // Assuming our maximum texture size is 4k, we'll never need more than 12 miplevels.
      return 12.0f;
  }
}

constexpr inline VkBool32 GetCompareEnable(SamplerCompareMode mode) noexcept {
  return mode == SamplerCompareMode::NONE ? VK_FALSE : VK_TRUE;
}

VulkanSamplerCache::VulkanSamplerCache() {}

VkSampler VulkanSamplerCache::GetSampler(SamplerParams params) noexcept {
  auto iter = cache_.find(params.u);
  if (iter != cache_.end()) {
    return iter->second;
  }

  VkSamplerCreateInfo sampler_info {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = GetFilter(params.filterMag),
    .minFilter = GetFilter(params.filterMin),
    .mipmapMode = GetMipmapMode(params.filterMin),
    .addressModeU = GetWrapMode(params.wrapS),
    .addressModeV = GetWrapMode(params.wrapT),
    .addressModeW = GetWrapMode(params.wrapR),
    .anisotropyEnable = params.anisotropyLog2 == 0 ? VK_FALSE : VK_TRUE,
    .maxAnisotropy = (float)(1u << params.anisotropyLog2),
    .compareEnable = GetCompareEnable(params.compareMode),
    .compareOp = VulkanUtils::GetCompareOp(params.compareFunc),
    .minLod = .0f,
    .maxLod = GetMaxLod(params.filterMin),
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE
  };
  VkSampler sampler ;
  VkResult res = vkCreateSampler(*VulkanContext::Get().device_,
                                 &sampler_info, nullptr,
                                 &sampler);
  CHECK_RESULT(res, "VulkanSampler", "CreateSampler Failed!");
  assert(res == VK_SUCCESS);
  cache_.insert({params.u, sampler});
  return sampler;
}

void VulkanSamplerCache::Reset() noexcept {
  for (auto pair : cache_) {
    vkDestroySampler(*VulkanContext::Get().device_, pair.second, nullptr);
  }
  cache_.clear();
}
}  // namespace our_graph