//
// Created by Glodxy on 2021/10/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSAMPLERCACHE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSAMPLERCACHE_H_
#include <map>

#include "VulkanDef.h"

namespace our_graph {
class VulkanSamplerCache {
 public:
  explicit VulkanSamplerCache();
  VkSampler GetSampler(SamplerParams params) noexcept;
  void Reset() noexcept;

 private:
  std::map<uint32_t, VkSampler > cache_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANSAMPLERCACHE_H_
