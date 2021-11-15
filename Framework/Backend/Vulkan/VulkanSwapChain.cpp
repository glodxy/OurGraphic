//
// Created by Glodxy on 2021/9/8.
//

#include "VulkanSwapChain.h"
#include "VulkanMemoryAllocator.h"
#include "VulkanMemoryHandle.h"
#include "VulkanCommands.h"
#include "VulkanContext.h"

our_graph::VulkanSwapChain::VulkanSwapChain(VkDevice device,
                                            VkInstance instance,
                                            IPlatform* platform,
                                            void* window_handle) {
  window_handle_ = window_handle;
  device_ = device;
  instance_ = instance;
  platform_ = platform;
  LOG_INFO("VulkanSwapChain", "physical_device:{}", (void*)(*VulkanContext::Get().physical_device_));
  physical_device_ = *(VulkanContext::Get().physical_device_);
  graphic_queue_family_idx_ = VulkanContext::Get().graphic_queue_family_idx_;
  if (!CreateSurface(window_handle_)) {
    LOG_ERROR("VulkanSwapChain", "Create failed!");
    return;
  }
  Create();
}

our_graph::VulkanSwapChain::~VulkanSwapChain() {
  Destroy();
  //销毁surface
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  surface_ = VK_NULL_HANDLE;
}

void our_graph::VulkanSwapChain::Create() {
  if (!CreateSwapChainExt()) {
    LOG_ERROR("VulkanSwapChain", "GetProcInstance Failed!");
    return;
  }

  if (!FindPresentationQueue()) {
    LOG_ERROR("VulkanSwapChain", "Find PresentQueue Failed!");
    return;
  }

  if (!CreateSwapChain()) {
    LOG_ERROR("VulkanSwapChain", "CreateSwapChain Failed");
    return;
  }
}

void our_graph::VulkanSwapChain::Destroy() {
  VulkanContext::Get().commands_->Commit();
  VulkanContext::Get().commands_->Wait();
  for (auto& swap_color :swapchain_images_) {
    if (!swapchain_) {
      vkDestroyImage(device_, swap_color.image, nullptr);
      vkFreeMemory(device_, swap_color.memory, nullptr);
    }
    vkDestroyImageView(device_, swap_color.view, nullptr);
    swap_color.view = VK_NULL_HANDLE;
  }

  vkDestroySwapchainKHR(device_, swapchain_, nullptr);
  swapchain_ = VK_NULL_HANDLE;

  vkDestroySemaphore(device_, image_available, nullptr);
  swapchain_images_.clear();

  vkDestroyImageView(device_, depth_.view, nullptr);
  vkDestroyImage(device_, depth_.image, nullptr);
  vkFreeMemory(device_, depth_.memory, nullptr);
  LOG_INFO("VulkanSwapChain", "Destroy Surface&SwapChain");
}


int our_graph::VulkanSwapChain::GetRenderTargetCnt() const {
  return 0;
}

bool our_graph::VulkanSwapChain::CreateSwapChainExt() {

  INSTANCE_FUNC_PTR(instance_, GetPhysicalDeviceSurfaceSupportKHR);
  CHECK_FUNC_PTR_RETURN(GetPhysicalDeviceSurfaceSupportKHR, false);

  INSTANCE_FUNC_AND_CHECK_RETURN(instance_, GetPhysicalDeviceSurfaceCapabilitiesKHR, false);
  INSTANCE_FUNC_AND_CHECK_RETURN(instance_, GetPhysicalDeviceSurfaceFormatsKHR, false);
  INSTANCE_FUNC_AND_CHECK_RETURN(instance_, GetPhysicalDeviceSurfacePresentModesKHR, false);
  INSTANCE_FUNC_AND_CHECK_RETURN(instance_, DestroySurfaceKHR, false);

  DEVICE_FUNC_AND_CHECK_RETURN(device_, CreateSwapchainKHR, false);
  DEVICE_FUNC_AND_CHECK_RETURN(device_, DestroySwapchainKHR, false);
  DEVICE_FUNC_AND_CHECK_RETURN(device_, GetSwapchainImagesKHR, false);
  DEVICE_FUNC_AND_CHECK_RETURN(device_, AcquireNextImageKHR, false);
  DEVICE_FUNC_AND_CHECK_RETURN(device_, QueuePresentKHR, false);

  return true;
}

bool our_graph::VulkanSwapChain::CreateSurface(
    void* handle) {
  VkSurfaceKHR surface;
  if (!platform_->CreateSurface(handle, instance_, 0, &surface)) {
    LOG_ERROR("VulkanSwapChain", "Create Surface failed!");
    return false;
  }
  surface_ = surface;
  return true;
}

bool our_graph::VulkanSwapChain::CreateSwapChain() {
  VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_param_);
  if (res != VK_SUCCESS) {
    LOG_ERROR("VulkanSwapChain", "GetSurfaceCap Failed! code:{}", res);
    return false;
  }

  const uint32_t max_image_cnt = surface_param_.maxImageCount;
  const uint32_t min_image_cnt = surface_param_.minImageCount;
  uint32_t desired_image_cnt = min_image_cnt + 1;

  // According to section 30.5 of VK 1.1, maxImageCount of zero means "that there is no limit on
  // the number of images, though there may be limits related to the total amount of memory used
  // by presentable images."
  if (max_image_cnt!= 0 && desired_image_cnt > max_image_cnt) {
    LOG_ERROR("VulkanSwapChain", "Swap chain does not support {} images.", desired_image_cnt);
    desired_image_cnt = surface_param_.minImageCount;
  }

  // 找到所有表面中支持RGBA格式的表面
  // 否则使用第一个表面
  if (surface_format_.format == VK_FORMAT_UNDEFINED) {

    uint32_t surface_formats_cnt;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_,
                                         &surface_formats_cnt, nullptr);

    surface_formats_.resize(surface_formats_cnt);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_,
                                         &surface_formats_cnt, surface_formats_.data());

    surface_format_ = surface_formats_[0];
    for (const VkSurfaceFormatKHR& format : surface_formats_) {
      if (format.format == VK_FORMAT_R8G8B8A8_UNORM) {
        surface_format_ = format;
      }
    }
  }
  // 获取交换链图片尺寸
  swapchain_image_size_ = surface_param_.currentExtent;

  // 获取交换链表面的混合模式
  const auto composition_caps = surface_param_.supportedCompositeAlpha;
  const auto composite_alpha = (composition_caps & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
                              VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  // 构建交换链创建信息
  VkSwapchainCreateInfoKHR swapchain_create_info {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface_,
      .minImageCount = desired_image_cnt,
      .imageFormat = surface_format_.format,
      .imageColorSpace = surface_format_.colorSpace,
      .imageExtent = swapchain_image_size_,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
          VK_IMAGE_USAGE_TRANSFER_DST_BIT | // Allows use as a blit destination.
          VK_IMAGE_USAGE_TRANSFER_SRC_BIT,  // Allows use as a blit source (for readPixels)


      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,

      .compositeAlpha = composite_alpha,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
      .clipped = VK_TRUE,

      .oldSwapchain = VK_NULL_HANDLE
  };
  res = vkCreateSwapchainKHR(device_, &swapchain_create_info, nullptr, &swapchain_);
  if (res != VK_SUCCESS) {
    LOG_ERROR("VulkanSwapChain", "CreateSwapchain Failed! code:{}", res);
    return false;
  }

  if (!BuildTexture()) {
    LOG_ERROR("VulkanSwapChain", "Generator TargetImage Failed!");
    return false;
  }

  // 创建信号量
  VkSemaphoreCreateInfo semaphore_create_info = {};
  semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  res = vkCreateSemaphore(device_, &semaphore_create_info, nullptr, &image_available);
  if (res != VK_SUCCESS) {
    LOG_ERROR("VulkanSwapChain", "CreateSemaphore Failed code:{}", res);
    return false;
  }

  const std::vector<VkFormat> depth_formats = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32};
  VkFormat depth_format = VulkanUtils::FindSupportedFormat(
      depth_formats, VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  if (!CreateDepthImage(depth_format, swapchain_image_size_)) {
    LOG_ERROR("VulkanSwapChain", "CreateDepthImage Failed!");
    return false;
  }
  return true;
}

bool our_graph::VulkanSwapChain::BuildTexture() {
  VkResult result = vkGetSwapchainImagesKHR(device_, swapchain_, &swapchain_image_cnt_available_, nullptr);
  if (result != VK_SUCCESS) {
    LOG_ERROR("VulkanSwapChain", "Get SwapChain ImageCnt Failed!");
    return false;
  }
  swapchain_images_.resize(swapchain_image_cnt_available_);

  // 临时存储Image，方便后面构建ImageView
  std::vector<VkImage> images(swapchain_image_cnt_available_);
  result = vkGetSwapchainImagesKHR(device_, swapchain_, &swapchain_image_cnt_available_, images.data());
  if (result != VK_SUCCESS) {
    LOG_ERROR("VulkanSwapChain", "Get SwapChain ImageCnt Failed. code:{}!", result);
    return false;
  }

  LOG_INFO("VulkanSwapChain", "CreateSwapChain:{},{},\n {},\n {},\n {},\n {}",
           swapchain_image_size_.width, swapchain_image_size_.height,
           surface_format_.format,
           surface_format_.colorSpace,
           swapchain_image_cnt_available_,
           surface_param_.currentTransform);

  // Create image views.
  VkImageViewCreateInfo image_view_create_info = {};
  image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_create_info.format = surface_format_.format;
  image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  image_view_create_info.subresourceRange.levelCount = 1;
  image_view_create_info.subresourceRange.layerCount = 1;
  for (size_t i = 0; i < images.size(); ++i) {
    image_view_create_info.image = images[i];
    VkImageView image_view;
    result = vkCreateImageView(device_, &image_view_create_info, nullptr,
                                       &image_view);
    if (result != VK_SUCCESS) {
      LOG_ERROR("VulkanSwapChain", "CreateImageView Failed! code:{}", result);
      return false;
    }
    swapchain_images_[i] = {
        .format = surface_format_.format,
        .image = images[i],
        .view = image_view,
        .memory = {},
        .texture = {},
        .layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
  }

  return true;
}

bool our_graph::VulkanSwapChain::CreateDepthImage(
    VkFormat format, Vec2i32 size) {
  // 创建Image
  VkImage depth_image;
  VkImageCreateInfo image_info {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = { size.width, size.height, 1 },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
  };
  VkResult res = vkCreateImage(device_, &image_info, nullptr, &depth_image);
  CHECK_RESULT(res, "VulkanSwapChain", "CreateDepthImage Failed!");
  assert(res == VK_SUCCESS);

  VkMemoryRequirements mem_reqs;
  vkGetImageMemoryRequirements(device_, depth_image, &mem_reqs);
  VkMemoryAllocateInfo allocInfo {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = mem_reqs.size,
      .memoryTypeIndex =VulkanUtils::SelectMemoryType(mem_reqs.memoryTypeBits,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
  };
  res = vkAllocateMemory(device_, &allocInfo, nullptr,
                                   &depth_.memory);
  CHECK_RESULT(res, "VulkanSwapChain", "Allocate Depth Memory Failed!");
  assert(res == VK_SUCCESS);
  res = vkBindImageMemory(device_, depth_image, depth_.memory, 0);
  CHECK_RESULT(res, "VulkanSwapChain", "Bind Depth Memory Failed!");
  assert(res == VK_SUCCESS);
  // 创建ImageView
  VkImageView depth_view;
  VkImageViewCreateInfo depth_view_info {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = depth_image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
          .levelCount = 1,
          .layerCount = 1,
      },
  };
  res = vkCreateImageView(device_, &depth_view_info, nullptr, &depth_view);
  CHECK_RESULT(res, "VulkanSwapChain", "Create Depth View Failed!");
  assert(res == VK_SUCCESS);

  depth_layout_ = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  depth_.view = depth_view;
  depth_.image = depth_image;
  depth_.format = format;
  depth_.layout = depth_layout_;


  auto& vk_cmd_buffer = VulkanContext::Get().commands_->Get();
  VkCommandBuffer cmd_buffer = vk_cmd_buffer.cmd_buffer_;
  // 创建布局
  VkImageMemoryBarrier barrier {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .newLayout = depth_layout_,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = depth_image,
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
          .levelCount = 1,
          .layerCount = 1,
      },
  };
  vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  return true;
}

bool our_graph::VulkanSwapChain::FindPresentationQueue() {
  uint32_t queue_family_cnt;
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device_, &queue_family_cnt, nullptr);
  std::vector<VkQueueFamilyProperties> props(queue_family_cnt);
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device_, &queue_family_cnt, props.data());

  uint32_t present_queue_family_idx = 0xffff;
  VkBool32 supported = VK_FALSE;
  vkGetPhysicalDeviceSurfaceSupportKHR(
      physical_device_, graphic_queue_family_idx_,
      surface_, &supported);
  if (supported) {
    present_queue_family_idx = graphic_queue_family_idx_;
  }

  // 当目前的不满足时，遍历寻找
  if (present_queue_family_idx == 0xffff) {
    for (int i = 0; i < queue_family_cnt; ++i) {
      vkGetPhysicalDeviceSurfaceSupportKHR(
          physical_device_, i,
          surface_, &supported);
      if (supported) {
        present_queue_family_idx = i;
        break;
      }
    }
  }

  //当当前的显示队列与满足交换链的队列不一致时
  if (graphic_queue_family_idx_ != present_queue_family_idx) {
    LOG_WARN("VulkanSwapChain", "Queue Not Equal!");
    // 使用新的queue
    vkGetDeviceQueue(device_, present_queue_family_idx,
                     0, &present_queue_);
  } else {
    present_queue_ = *(VulkanContext::Get().graphic_queue_);
  }

  return true;
}

bool our_graph::VulkanSwapChain::Acquire() {
  VkResult res= vkAcquireNextImageKHR(device_, swapchain_,
                                      UINT64_MAX, image_available,
                                      VK_NULL_HANDLE, &current_idx_);

  if (res == VK_SUBOPTIMAL_KHR && !sub_optimal_) {
    LOG_WARN("VulkanSwapChain", "Use Suboptimal SwapChain!");
    sub_optimal_ = true;
  }

  VulkanContext::Get().commands_->InjectDependency(image_available);
  acquired_ = true;
  assert(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);
  return true;
}

void our_graph::VulkanSwapChain::MakePresentable() {
  VulkanAttachment& current_color = swapchain_images_[current_idx_];
  VkImageMemoryBarrier barrier {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .dstAccessMask = 0,
    .oldLayout = first_render_pass_ ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .newLayout = current_color.layout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = current_color.image,
    .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .levelCount = 1,
        .layerCount = 1,
    },
  };
  VkCommandBuffer cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;
  vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr,
                       0, nullptr, 1, &barrier);
}

bool our_graph::VulkanSwapChain::HasResized() const {
  if (surface_ == VK_NULL_HANDLE) {
    return false;
  }
  VkSurfaceCapabilitiesKHR surface_capabilities_khr;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*VulkanContext::Get().physical_device_,
                                            surface_, &surface_capabilities_khr);
  return swapchain_image_size_.width != surface_capabilities_khr.currentExtent.width ||
          swapchain_image_size_.height != surface_capabilities_khr.currentExtent.height;
}