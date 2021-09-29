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

// todo:修改接口后还未修改实现
bool VulkanPlatformMacos::CreateSurface(void *native_window, void *instance,
                                        uint64_t flags, void* surface) {
  // Obtain the CAMetalLayer-backed view.
  NSWindow* ns_window = (__bridge NSWindow*) native_window;
  [ns_window setColorSpace:[NSColorSpace sRGBColorSpace]];
  NSView* nsview = [ns_window contentView];
  assert([nsview isKindOfClass:[NSView class]]);

  if (![nsview.layer isKindOfClass:[CAMetalLayer class]])
  {
    [nsview setWantsLayer:YES];
    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.bounds = nsview.bounds;

    // It's important to set the drawableSize to the actual backing pixels. When rendering
    // full-screen, we can skip the macOS compositor if the size matches the display size.
    metalLayer.drawableSize = [nsview convertSizeToBacking:nsview.bounds.size];

    // In its implementation of vkGetPhysicalDeviceSurfaceCapabilitiesKHR, MoltenVK takes into
    // consideration both the size (in points) of the bounds, and the contentsScale of the
    // CAMetalLayer from which the Vulkan surface was created.
    // See also https://github.com/KhronosGroup/MoltenVK/issues/428
    metalLayer.contentsScale = nsview.window.backingScaleFactor;

    // This is set to NO by default, but is also important to ensure we can bypass the compositor
    // in full-screen mode
    // See "Direct to Display" http://metalkit.org/2017/06/30/introducing-metal-2.html.
    metalLayer.opaque = YES;

    [nsview setLayer:metalLayer];
  }
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