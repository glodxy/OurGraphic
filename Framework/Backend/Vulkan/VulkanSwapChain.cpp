//
// Created by Glodxy on 2021/9/8.
//

#include "VulkanSwapChain.h"

bool our_graph::VulkanSwapChain::CreateSwapChainExt() {

  INSTANCE_FUNC_PTR(instance_, GetPhysicalDeviceSurfaceSupportKHR);
  CHECK_FUNC_PTR_RETURN(GetPhysicalDeviceSurfaceSupportKHR, false);

  INSTANCE_FUNC_AND_CHECK_RETURN(instance_, GetPhysicalDeviceSurfaceCapabilitiesKHR, false);

}