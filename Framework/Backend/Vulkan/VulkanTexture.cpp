//
// Created by Glodxy on 2021/10/5.
//

#include "Utils/OGLogging.h"
#include "VulkanTexture.h"
#include "VulkanContext.h"
#include "Framework/Backend/include/BackendUtils.h"
#include "../DataReshaper.h"

namespace our_graph {

static void TransitionImageLayout(VkCommandBuffer cmd, VkImage image,
                                  VkImageLayout old_layout, VkImageLayout new_layout,
                                  uint32_t mip_level, uint32_t layer_count,
                                  uint32_t level_count, VkImageAspectFlags aspect) {
  if (old_layout == new_layout) {
    return;
  }

  VkImageMemoryBarrier barrier {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .oldLayout = old_layout,
    .newLayout = new_layout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = image,
    .subresourceRange = {
        .aspectMask = aspect,
        .baseMipLevel = mip_level,
        .levelCount = level_count,
        .baseArrayLayer = 0,
        .layerCount = layer_count,
    },
  };
  VkPipelineStageFlags  src_stage;
  VkPipelineStageFlags  dst_stage;
  switch (new_layout) {
    // 要传输数据至该image
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;

    // 读取image的数据到CPU
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;
    // 设置该image的状态为传入shader
    // 上一个状态一定为数据传输
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_GENERAL:
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      break;

    // 设置该image的状态为显示
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.dstAccessMask = 0;
      src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      dst_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      break;

    default:
      LOG_ERROR("VulkanTexture", "Unsupported layout transition!");
  }
  vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);
}

VulkanTexture::VulkanTexture(SamplerType sampler_type,
                             uint8_t levels,
                             TextureFormat format,
                             uint8_t samples,
                             uint32_t width,
                             uint32_t height,
                             uint32_t depth,
                             TextureUsage usage,
                             VulkanStagePool &stage_pool,
                             VkComponentMapping swizzle) :
    ITexture(sampler_type, levels, samples, width, height, depth, format, usage),
    vk_format_(format == TextureFormat::DEPTH24 ?
              VulkanContext::Get().final_depth_format_ :
              VulkanUtils::GetVkFormat(format)),
    swizzle_(swizzle), stage_pool_(stage_pool) {
  VkImageCreateInfo image_info {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = sampler_type == SamplerType::SAMPLER_3D ?
        VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
    .format = vk_format_,
    .extent = {width_, height_, depth_},
    .mipLevels = levels_,
    .arrayLayers = 1,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = 0
  };

  // 为立方体纹理时，需要定义为6个面的纹理数组
  if (sampler_type == SamplerType::SAMPLER_CUBEMAP) {
    image_info.arrayLayers = 6;
    image_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  }
  // 此处采用三维纹理的方式来模拟纹理数组
  if (sampler_type == SamplerType::SAMPLER_2D_ARRAY) {
    image_info.arrayLayers = depth_;
    image_info.extent.depth = 1;
  }

  const VkImageUsageFlags  blittable = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  // 判断该纹理是否会用于采样
  if ((static_cast<uint8_t>(usage_) &
       static_cast<uint8_t>(TextureUsage::SAMPLEABLE)) != 0) {
    // 如果会用于采样
#if  DEBUG
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(*VulkanContext::Get().physical_device_,
                                        vk_format_, &props);
    if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
      LOG_WARN("VulkanTexture", "Texture usage is SAMPLEABLE but format {} "
                                "not sampleable with optimal tiling!", vk_format_);
    }
#endif
    image_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }

  // 判断是否会用于render target
  if ((static_cast<uint8_t>(usage_) &
      static_cast<uint8_t>(TextureUsage::COLOR_ATTACHMENT)) != 0) {
    image_info.usage |= (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | blittable);
    // 判断是否会用作subpass的输入
    if ((static_cast<uint8_t>(usage_) &
        static_cast<uint8_t>(TextureUsage::SUBPASS_INPUT)) != 0) {
      image_info.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
  }

  if ((static_cast<uint8_t>(usage_) &
      static_cast<uint8_t>(TextureUsage::STENCIL_ATTACHMENT)) != 0) {
    image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }

  // 是否可写
  if ((static_cast<uint8_t>(usage_) &
      static_cast<uint8_t>(TextureUsage::UPLOADABLE)) != 0) {
    image_info.usage |= blittable;
  }

  // 是否会作为深度的render target
  if ((static_cast<uint8_t>(usage_) &
      static_cast<uint8_t>(TextureUsage::DEPTH_ATTACHMENT)) != 0) {
    image_info.usage |= blittable;
    image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // 如果采样数大于1
    if (samples > 1) {
      image_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
  }

  /**
   * 约束纹理的采样数在设备支持的范围内
   * */
  const auto& limits = VulkanContext::Get().physical_device_properties_.limits;
  // 普通采样
  if (image_info.usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
    samples = VulkanUtils::ReduceSampleCount(samples, VulkanUtils::IsDepthFormat(vk_format_) ?
                                                limits.sampledImageDepthSampleCounts : limits.sampledImageColorSampleCounts);
  }
  // 用作render target
  if (image_info.usage & (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
    samples = VulkanUtils::ReduceSampleCount(samples, limits.framebufferDepthSampleCounts &
                                              limits.framebufferColorSampleCounts);
  }
  samples_ = samples;
  image_info.samples = (VkSampleCountFlagBits)samples;

  VkResult res = vkCreateImage(*VulkanContext::Get().device_,
                               &image_info, nullptr, &texture_image_);
  if (res != VK_SUCCESS) {
    LOG_ERROR("VulkanTexture", "CreateImage Failed! code:{}", res);
  }
  LOG_INFO("VulkanTexture", "CreateImage :\n"
                            "handle = {}\n"
                            "extent = [{}, {}, {}]\n"
                            "miplevels = {}\n"
                            "usage = {}\n"
                            "samples = {}\n"
                            "format = {}",
           (void*)texture_image_,
           width_, height_, depth_,
           levels_,
           image_info.usage,
           image_info.samples,
           image_info.format);

  // 分配空间
  VkMemoryRequirements mem_reqs = {};
  vkGetImageMemoryRequirements(*VulkanContext::Get().device_, texture_image_, &mem_reqs);
  VkMemoryAllocateInfo alloc_info {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = mem_reqs.size,
    .memoryTypeIndex = VulkanUtils::SelectMemoryType(mem_reqs.memoryTypeBits,
                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
  };
  res = vkAllocateMemory(*VulkanContext::Get().device_, &alloc_info,
                         nullptr, &texture_image_memory_);
  CHECK_RESULT(res, "VulkanTexture", "AllocateMemory Failed!");
  assert(res == VK_SUCCESS);
  res = vkBindImageMemory(*VulkanContext::Get().device_, texture_image_, texture_image_memory_, 0);
  CHECK_RESULT(res, "VulkanTexture", "BindMemory Failed!");
  assert(res == VK_SUCCESS);

  aspect_ = (static_cast<uint8_t>(usage) &
              static_cast<uint8_t>(TextureUsage::DEPTH_ATTACHMENT)) != 0 ?
                  VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

  primary_view_range_.aspectMask = aspect_;
  primary_view_range_.baseMipLevel = 0;
  primary_view_range_.levelCount = levels_;
  primary_view_range_.baseArrayLayer = 0;
  if (sampler_type == SamplerType::SAMPLER_CUBEMAP) {
    view_type_ = VK_IMAGE_VIEW_TYPE_CUBE;
    primary_view_range_.layerCount = 6;
  } else if (sampler_type == SamplerType::SAMPLER_2D_ARRAY) {
    view_type_ = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    primary_view_range_.layerCount = depth_;
  } else if (sampler_type == SamplerType::SAMPLER_3D) {
    view_type_ = VK_IMAGE_VIEW_TYPE_3D;
    primary_view_range_.layerCount = 1;
  } else {
    view_type_ = VK_IMAGE_VIEW_TYPE_2D;
    primary_view_range_.layerCount = 1;
  }

  GetImageView(primary_view_range_);

  if ((static_cast<uint8_t>(usage_) &
      (static_cast<uint8_t>(TextureUsage::COLOR_ATTACHMENT) |
       static_cast<uint8_t>(TextureUsage::DEPTH_ATTACHMENT)))) {
    uint32_t layers = primary_view_range_.layerCount;
    TransitionImageLayout(VulkanContext::Get().commands_->Get().cmd_buffer_,
                          texture_image_, VK_IMAGE_LAYOUT_UNDEFINED,
                          VulkanUtils::GetTextureLayout(usage), 0,
                          layers, levels, aspect_);
    LOG_INFO("VulkanTexture", "TransitionImageLayout!");
  }
}


VulkanTexture::~VulkanTexture() {
  delete msaa_;
  vkDestroyImage(*VulkanContext::Get().device_,
                 texture_image_, nullptr);
  vkFreeMemory(*VulkanContext::Get().device_,
               texture_image_memory_, nullptr);
  for (auto entry : cached_image_views_) {
    vkDestroyImageView(*VulkanContext::Get().device_, entry.second, nullptr);
  }
}

void VulkanTexture::Update2DImage(const PixelBufferDescriptor &data, uint32_t width, uint32_t height, int mip_levels) {
  Update3DImage(std::move(data), width, height, 1, mip_levels);
}

void VulkanTexture::Update3DImage(const PixelBufferDescriptor &data,
                                  uint32_t width,
                                  uint32_t height,
                                  uint32_t depth,
                                  int mip_levels) {
  assert(width <= width_ && height <= height_ && depth <= depth_);
  const PixelBufferDescriptor* host_data = &data;
  PixelBufferDescriptor reshape_data;

  if (BackendUtil::Reshape(data, reshape_data)) {
    host_data = &reshape_data;
  }

  const VkFormat host_format = VulkanUtils::GetVkFormat(host_data->format_, host_data->type_);
  const VkFormat device_format = VulkanUtils::GetVkFormatLinear(vk_format_);

  // 格式相同直接贴，不相同则逐字节拷贝
  if (host_format != device_format && host_format != VK_FORMAT_UNDEFINED) {
    UpdateWithBlitImage(*host_data, width, height, depth, mip_levels);
  } else {
    UpdateWithCopyBuffer(*host_data, width, height, depth, mip_levels);
  }
}

void VulkanTexture::UpdateCubeImage(const PixelBufferDescriptor &data,
                                    const FaceOffsets &face_offsets,
                                    int mip_levels) {
  assert(sampler_type_ == SamplerType::SAMPLER_CUBEMAP);
  const bool reshape = VulkanUtils::GetBytesPerPixel(format_) == 3;
  const void* cpu_data = data.buffer_;
  const uint32_t num_src_bytes = data.size_;
  const uint32_t num_dst_bytes = reshape ? (4 * num_src_bytes / 3) : num_src_bytes;

  const VulkanStage* stage = stage_pool_.AcquireStage(num_dst_bytes);
  FaceOffsets offsets = face_offsets;
  void* mapped;
  vmaMapMemory(VulkanContext::Get().allocator_, stage->memory, &mapped);
  if (reshape) {
    DataReshaper::reshape<uint8_t, 3, 4>(mapped, cpu_data, num_src_bytes);
    //! reshape后需要重新计算offset
    for (int i = 0; i < 6; ++i) {
      offsets[i] = width_ * height_ * sizeof(uint32_t) * i;
    }
  } else {
    memcpy(mapped, cpu_data, num_src_bytes);
  }
  vmaUnmapMemory(VulkanContext::Get().allocator_, stage->memory);
  vmaFlushAllocation(VulkanContext::Get().allocator_, stage->memory, 0, num_dst_bytes);

  const VkCommandBuffer cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;
  const uint32_t width = std::max(1u, width_ >> mip_levels);
  const uint32_t height = std::max(1u, height_ >> mip_levels);

  TransitionImageLayout(cmd_buffer, texture_image_, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels, 6, 1,aspect_);
  CopyBufferToImage(cmd_buffer, stage->buffer, texture_image_, width_, height_, 1,
                    &offsets, mip_levels);
  TransitionImageLayout(cmd_buffer, texture_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VulkanUtils::GetTextureLayout(usage_), mip_levels,
                        6, 1, aspect_);
}

void VulkanTexture::UpdateWithCopyBuffer(const PixelBufferDescriptor &host_data,
                                         uint32_t width,
                                         uint32_t height,
                                         uint32_t depth,
                                         int mip_level) {
  void* mapped = nullptr;
  const VulkanStage* stage = stage_pool_.AcquireStage(host_data.size_);
  vmaMapMemory(VulkanContext::Get().allocator_, stage->memory, &mapped);
  memcpy(mapped, host_data.buffer_, host_data.size_);
  vmaUnmapMemory(VulkanContext::Get().allocator_, stage->memory);
  vmaFlushAllocation(VulkanContext::Get().allocator_, stage->memory, 0, host_data.size_);

  const VkCommandBuffer cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;
  TransitionImageLayout(cmd_buffer, texture_image_, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_level, 1, 1, aspect_);

  CopyBufferToImage(cmd_buffer, stage->buffer, texture_image_, width_, height_,
                    depth_, nullptr, mip_level);

  TransitionImageLayout(cmd_buffer, texture_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VulkanUtils::GetTextureLayout(usage_), mip_level, 1, 1, aspect_);
}

void VulkanTexture::UpdateWithBlitImage(const PixelBufferDescriptor &host_data,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t depth,
                                        int mip_level) {
  void* mapped = nullptr;
  const VulkanStageImage* stage = stage_pool_.AcquireImage(host_data.format_,
                                                      host_data.type_,
                                                      width, height);
  vmaMapMemory(VulkanContext::Get().allocator_, stage->memory, &mapped);
  memcpy(mapped, host_data.buffer_, host_data.size_);
  vmaUnmapMemory(VulkanContext::Get().allocator_, stage->memory);
  vmaFlushAllocation(VulkanContext::Get().allocator_, stage->memory, 0,
                     host_data.size_);

  const VkCommandBuffer cmd_buffer = VulkanContext::Get().commands_->Get().cmd_buffer_;

  const int layer = 0;

  const VkOffset3D rect[2] { {0, 0, 0}, {int32_t(width), int32_t(height), 1} };

  const VkImageBlit blit_region[1] = {{
    .srcSubresource = {aspect_, 0, 0, 1},
    .srcOffsets = {rect[0], rect[1]},
    .dstSubresource = {aspect_, uint32_t (mip_level), layer, 1},
    .dstOffsets = {rect[0], rect[1]}
  }};

  TransitionImageLayout(cmd_buffer, stage->image, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mip_level, 1, 1, aspect_);
  TransitionImageLayout(cmd_buffer, texture_image_, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_level, 1, 1, aspect_);
  vkCmdBlitImage(cmd_buffer, stage->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                 texture_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                 blit_region, VK_FILTER_NEAREST);
  TransitionImageLayout(cmd_buffer, texture_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VulkanUtils::GetTextureLayout(usage_), mip_level, 1, 1, aspect_);
}

void VulkanTexture::SetPrimaryRange(uint32_t min_mip_level, uint32_t max_mip_level) {
  max_mip_level = std::min(int(max_mip_level), int(levels_ - 1));
  primary_view_range_.baseMipLevel = min_mip_level;
  primary_view_range_.levelCount = max_mip_level - min_mip_level + 1;
  GetImageView(primary_view_range_);
}

VkImageView VulkanTexture::GetAttachmentView(int single_level, int single_layer, VkImageAspectFlags aspect) {
  return GetImageView({
    .aspectMask = aspect,
    .baseMipLevel = uint32_t(single_level),
    .levelCount = uint32_t(1),
    .baseArrayLayer = uint32_t(single_layer),
    .layerCount = uint32_t(1),
  }, true);
}

VkImageView VulkanTexture::GetImageView(VkImageSubresourceRange range, bool is_attachment) {
  auto iter = cached_image_views_.find(range);
  if (iter != cached_image_views_.end()) {
    return iter->second;
  }
  VkImageViewCreateInfo view_info {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .image = texture_image_,
    .viewType = is_attachment ? VK_IMAGE_VIEW_TYPE_2D : view_type_,
    .format = vk_format_,
    .components = is_attachment ? (VkComponentMapping{}) : swizzle_,
    .subresourceRange = range,
  };
  VkImageView view;
  vkCreateImageView(*VulkanContext::Get().device_, &view_info, nullptr, &view);
  cached_image_views_.emplace(range, view);
  return view;
}

void VulkanTexture::CopyBufferToImage(VkCommandBuffer cmd,
                                      VkBuffer buffer,
                                      VkImage image,
                                      uint32_t width,
                                      uint32_t height,
                                      uint32_t depth,
                                      const FaceOffsets *face_offsets,
                                      int mip_level) {
  VkExtent3D extent {width, height, depth};
  if (sampler_type_ == SamplerType::SAMPLER_CUBEMAP) {
    assert(face_offsets != nullptr);
    VkBufferImageCopy regions[6] = {{}};
    for (int i = 0; i < 6; ++i) {
      auto& region = regions[i];
      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.baseArrayLayer = i;
      region.imageSubresource.layerCount = 1;
      region.imageSubresource.mipLevel = mip_level;
      region.imageExtent = extent;
      region.bufferOffset = face_offsets->offsets[i];
    }
    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, regions);
    return;
  }
  VkBufferImageCopy region = {};
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = mip_level;
  region.imageSubresource.layerCount = 1;
  region.imageExtent = extent;
  vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

}  // namespace our_graph