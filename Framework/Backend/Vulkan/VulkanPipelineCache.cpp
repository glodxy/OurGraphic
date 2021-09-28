//
// Created by Glodxy on 2021/9/26.
//

#include "VulkanPipelineCache.h"

#include "vk_mem_alloc.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"


namespace our_graph {
// 创建默认的管线状态
static VulkanPipelineCache::RasterState CreateDefaultRasterState() {
  return VulkanPipelineCache::RasterState {
    .rasterization = {
        .depth_clamp_enable = VK_FALSE,
        .rasterizer_discard_enable = VK_FALSE,
        .polygon_mode = VK_POLYGON_MODE_FILL, // 填充模式
        .cull_mode = VK_CULL_MODE_NONE, // 不裁剪
        .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE, // 逆时针为前向面
        .depth_bias_enable = VK_FALSE,
        .depth_bias_constant_factor = .0f,
        .depth_bias_clamp = .0f,
        .depth_bias_slope_factor = .0f,
        .line_width = 1.f
    },
    .blending = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = 0xf, // 全写
    },
    .depth_stencil = {
        .depth_test_enable = VK_TRUE,
        .depth_write_enable = VK_TRUE,
        .depth_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL, // 小于等于则通过深度
        .depth_bounds_test_enable = VK_FALSE, // 不开启边界测试
        .stencil_test_enable = VK_FALSE, // 不开启模板测试
    },
    .multisampling = {
        .rasterization_samples = VK_SAMPLE_COUNT_1_BIT, // 只采样1位
        .alpha_to_coverage_enable = true,
    },
    .color_target_count = 1, // 默认只有一个render target
  };
}

VulkanPipelineCache::VulkanPipelineCache() :
  default_raster_state_(CreateDefaultRasterState()) {
  MarkDirtyDescriptor();
  MarkDirtyPipeline();

  current_descriptor_ = {};
  current_pipeline_ = {};

  dummy_buffer_write_info_.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  dummy_buffer_write_info_.pNext = nullptr;
  dummy_buffer_write_info_.dstArrayElement = 0;
  dummy_buffer_write_info_.descriptorCount = 1; //每个set只有一个Descriptor
  dummy_buffer_write_info_.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  dummy_buffer_write_info_.pImageInfo = nullptr;
  dummy_buffer_write_info_.pBufferInfo = &dummy_buffer_info_;
  dummy_buffer_write_info_.pTexelBufferView = nullptr;

  dummy_sampler_write_info_.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  dummy_sampler_write_info_.pNext = nullptr;
  dummy_sampler_write_info_.dstArrayElement = 0;
  dummy_sampler_write_info_.descriptorCount = 1;
  dummy_sampler_write_info_.pImageInfo = &dummy_sampler_info_;
  dummy_sampler_write_info_.pBufferInfo = nullptr;
  dummy_sampler_write_info_.pTexelBufferView = nullptr;
  dummy_sampler_write_info_.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

  dummy_target_info_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  dummy_target_write_info_.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  dummy_target_write_info_.pNext = nullptr;
  dummy_target_write_info_.dstArrayElement = 0;
  dummy_target_write_info_.descriptorCount = 1;
  dummy_target_write_info_.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
  dummy_target_write_info_.pImageInfo = &dummy_target_info_;
  dummy_target_write_info_.pBufferInfo = nullptr;
  dummy_target_write_info_.pTexelBufferView = nullptr;
}

VulkanPipelineCache::~VulkanPipelineCache() noexcept {
  DestroyAllCache();
}

void VulkanPipelineCache::SetDevice(VkDevice device, VmaAllocator allocator) {
  assert(device == VK_NULL_HANDLE);

  device_ = device;
  allocator_ = allocator;

  // 创建 dummy buffer
  VkBufferCreateInfo buffer_info {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = 16,
    .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  };
  VmaAllocationCreateInfo alloc_info {
    .usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };
  vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &dummy_buffer_,
                  &dummy_memory_, nullptr);
  dummy_buffer_info_.buffer = dummy_buffer_;
  dummy_buffer_info_.range = buffer_info.size;
}

bool VulkanPipelineCache::BindDescriptors(VkCommandBuffer cmd_buffer) {
  VkDescriptorSet descriptor_set[VulkanPipelineCache::DESCRIPTOR_TYPE_COUNT];
  bool bind = false, overflow = false;
  GetOrCreateDescriptors(descriptor_set, &bind, &overflow);

  if (overflow) {
    return false;
  }

  if (bind) {
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout_, 0, VulkanPipelineCache::DESCRIPTOR_TYPE_COUNT,
                            descriptor_set, 0, nullptr);
  }
  return true;
}

void VulkanPipelineCache::BindPipeline(VkCommandBuffer cmd_buffer) noexcept {
  VkPipeline pipeline;
  if (GetOrCreatePipeline(&pipeline)) {
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }
}

void VulkanPipelineCache::BindScissor(VkCommandBuffer cmd_buffer, VkRect2D scissor) noexcept {
  VkRect2D& current_scissor = cmd_buffer_state_[current_cmd_buffer_].scissor;
  if (!VulkanUtils::Equal(current_scissor, scissor)) {
    current_scissor = scissor;
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
  }
}

void VulkanPipelineCache::GetOrCreateDescriptors(
    VkDescriptorSet *descriptors,
    bool *bind, bool *overflow) noexcept {
  // 如果是第一次调用，需要创建新的layout
  if (!pipeline_layout_) {
    CreateLayoutsAndDescriptors();
  }

  // 获取当前command buffer的Descriptor集合
  DescriptorBundle*& descriptor_bundle =
      cmd_buffer_state_[current_cmd_buffer_].current_descriptor_bundle;

  // 如果没被改变，代表没有经过绑定，
  // 直接重置该command buffer的Descriptor
  if (!VulkanUtils::GetBit(dirty_descriptor_,current_cmd_buffer_)) {
    assert(descriptor_bundle);
    for (int i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
      descriptors[i] = descriptor_bundle->handles[i];
    }
    VulkanUtils::SetBit(&descriptor_bundle->command_buffers, current_cmd_buffer_);
  }


}

}  // namespace our_graph