//
// Created by Glodxy on 2021/9/1.
//
#include "../include/TextureTypedef.h"
#include "VulkanTexture.h"

#include "VulkanTextureBuffer.h"
#include "VulkanTextureView.h"
#include "VulkanDevice.h"
#include "Utils/OGLogging.h"

our_graph::VulkanTexture::VulkanTexture(const std::string &name,
                                        VkDevice device,
                                        VkImage image,
                                        VkImageView view,
                                        bool need_buffer) : name_(name),
                                        device_(device) {
  buffer_ = std::make_shared<VulkanTextureBuffer>(name, device, image, need_buffer);
  descriptor_ = std::make_shared<VulkanTextureView>(device, view);
}

our_graph::VulkanTexture::VulkanTexture(const std::string &name,
                                        VkDevice device,
                                        VkImageCreateInfo image_create_info,
                                        VkImageViewCreateInfo view_create_info,
                                        uint64_t memory_flag_bits) :
                                        name_(name),
                                        device_(device) {
  buffer_ = std::make_shared<VulkanTextureBuffer>(name_, device_,
                                                  image_create_info,
                                                  memory_flag_bits);

  VkImage* image = (VkImage*)buffer_->GetInstance();
  view_create_info.image = *image;
  descriptor_ =std::make_shared<VulkanTextureView>(device_, view_create_info);
}

our_graph::VulkanTexture::~VulkanTexture() noexcept {
  descriptor_->Destroy();
  buffer_ = nullptr;
  LOG_INFO("VulkanTexture", "DestroyTexture:{}", name_);
}

void our_graph::VulkanTexture::Create() {

}

void our_graph::VulkanTexture::Destroy() {

}



VkImageCreateInfo our_graph::VulkanTexture::GetImageCreateInfo() const {
  VkImageCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.format = VK_FORMAT_R8G8B8A8_UNORM;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  info.extent = {1024, 860, 1};
  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.queueFamilyIndexCount = 0;
  info.pQueueFamilyIndices = nullptr;
  info.mipLevels = 1;
  info.arrayLayers = 1;
  return info;
}

VkImageViewCreateInfo our_graph::VulkanTexture::GetImageViewCreateInfo() const {
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = nullptr;
  viewInfo.flags = 0;
  viewInfo.image = *(VkImage*)buffer_->GetInstance();
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;
  return viewInfo;
}

std::shared_ptr<our_graph::IDescriptor> our_graph::VulkanTexture::GetView() {
  return descriptor_;
}

std::shared_ptr<our_graph::IBuffer> our_graph::VulkanTexture::GetBuffer() {
  return buffer_;
}