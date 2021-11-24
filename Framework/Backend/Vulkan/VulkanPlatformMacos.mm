//
// Created by Glodxy on 2021/8/29.
//

#include "VulkanPlatformMacos.h"
#include "VulkanDef.h"

#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

namespace our_graph {
std::vector<const char *> VulkanPlatformMacos::GetInstanceExtLayers() const {
  static std::vector<const char*> res = {
    "VK_MVK_macos_surface"
  };
  return res;
}

bool VulkanPlatformMacos::CreateSurface(void *native_window, void *instance,
                                        uint64_t flags, void* surface) {
  // Obtain the CAMetalLayer-backed view.
  NSView* nsview = (__bridge NSView*) native_window;
  if (!nsview) {
    LOG_ERROR("VulkanPlatformMacos","cannot get Cocoa NsView!");
  }

  // Create the VkSurface.
  VkSurfaceKHR* vk_surface = (VkSurfaceKHR*) surface;
  VkMacOSSurfaceCreateInfoMVK createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
  createInfo.pView = (__bridge void*) nsview;
  VkResult result = vkCreateMacOSSurfaceMVK((VkInstance) instance, &createInfo, nullptr, vk_surface);
  if (result != VK_SUCCESS) {
    LOG_ERROR("VulkanPlatformMacos", "Create Macos Surface Failed!");
    return false;
  }

  return true;
}


}  // namespace our_graph