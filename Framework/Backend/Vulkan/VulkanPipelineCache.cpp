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

/**
 * 会根据当前的资源自行生成Descriptor来进行绑定
 * */
bool VulkanPipelineCache::BindDescriptors(VkCommandBuffer cmd_buffer) {
  VkDescriptorSet descriptor_set[VulkanPipelineCache::DESCRIPTOR_TYPE_COUNT];
  bool bind = false, overflow = false;
  overflow = GetOrCreateDescriptors(descriptor_set, &bind);

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
  bool bind = false;
  if (GetOrCreatePipeline(&pipeline, &bind) && bind) {
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

/**
 * Descriptor创建以及绑定策略：
 * 1.检查对应的CommandBuffer是否有修改Descriptor
 * 脏位数据为0，代表没有修改，为1，代表有修改
 * 在初始化时会全部置1，确保每个CommandBuffer都能重新绑定
 * 没有修改会直接返回
 *
 * 2.发生改变了，首先查看cache中有无已设定好的descriptor bundle
 * 如果有则直接使用现成的bundle，在外部重新绑定至流水线
 *
 * 3.如果cache不存在，则首先检查缓冲区是否有未生成的descriptor sets
 * 如果有则取出这些sets，以进行后续操作
 * 否则尝试从DescriptorSetPool中分配一个Sets（由3个Set构成）
 *
 * 4.将新生成的Set加入cache
 *
 * 5.检查dummy资源是否存在，不存在则生成（该资源在整个流程中只需生成一次，做默认属性的作用）
 *
 * 6.将新的Set更新至Vulkan（根据当前的资源来设置属性）
 * */
bool VulkanPipelineCache::GetOrCreateDescriptors(
    VkDescriptorSet *descriptors,
    bool *bind) noexcept {
  *bind = false;
  // 如果是第一次调用，需要创建新的layout
  if (!pipeline_layout_) {
    CreateLayoutsAndDescriptors();
  }

  // 获取当前command buffer的Descriptor集合
  DescriptorBundle*& descriptor_bundle =
      cmd_buffer_state_[current_cmd_buffer_].current_descriptor_bundle;

  // 该位为0，代表没有发生改变，不需要重新绑定
  // 直接将当前的descriptorSet传出去
  if (!VulkanUtils::GetBit(dirty_descriptor_,current_cmd_buffer_)) {
    assert(descriptor_bundle);
    for (int i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
      descriptors[i] = descriptor_bundle->handles[i];
    }
    // 设置标记，代表该command buffer已被绑定
    VulkanUtils::SetBit(&descriptor_bundle->command_buffers, current_cmd_buffer_);
    return true;
  }

  // 查找缓存的对象，如果存在则复用
  auto iter = descriptors_.find(current_descriptor_);
  if (iter != descriptors_.end()) {
    descriptor_bundle = &iter->second;
    // 设置当前的descriptorSet，
    for (int i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
      descriptors[i] = descriptor_bundle->handles[i];
    }
    // 设置command buffer的脏位
    VulkanUtils::SetBit(&descriptor_bundle->command_buffers, current_cmd_buffer_);
    // 设置DescriptorSet的脏位，代表该command buffer的Descriptor已经设置了
    VulkanUtils::SetBit(&dirty_descriptor_, current_cmd_buffer_, 0);
    *bind = true;
    return true;
  }

  // cache中不存在
  // 先查看缓冲区，如果有直接使用缓冲区数据
  if (!descriptor_sets_[0].empty()) {
    for (int i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
      descriptors[i] = descriptor_sets_[i].back();
      descriptor_sets_[i].pop_back();
    }
  } else {
    // 没有则临时创建资源
    // 如果当前cache的大小大于或等于池的大小，则需要先对池扩容
    if (descriptors_.size() >= descriptor_pool_size_) {
      GrowDescriptorPool();
    }

    VkDescriptorSetAllocateInfo alloc_info = {};

    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool_;
    alloc_info.descriptorSetCount = DESCRIPTOR_TYPE_COUNT; // 表示有几种Descriptor
    alloc_info.pSetLayouts = descriptor_set_layouts_; // 这几种Descriptor的layout

    // 在此处给外部传入的descriptors分配DescriptorSets，此处为3个
    VkResult res = vkAllocateDescriptorSets(device_, &alloc_info, descriptors);
    if (res != VK_SUCCESS) {
      LOG_ERROR("VulkanPipeline", "Allocate DescriptorSets Failed! code:{}", res);
      return false;
    }
  }

  // 走到此处说明资源没在cache中，需要构造bundle将其加入cache
  descriptors_.emplace(std::make_pair(current_descriptor_, DescriptorBundle {}));

  // 此处设置了当前command buffer的descriptor bundle
  auto& bundle = descriptors_.at(current_descriptor_);
  descriptor_bundle = &bundle;
  for (int i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
    descriptor_bundle->handles[i] = descriptors[i];
  }
  //todo:待定
  descriptor_bundle->command_buffers = (1 << current_cmd_buffer_);

  VulkanUtils::SetBit(&dirty_descriptor_, current_cmd_buffer_, 0);


  //todo：解释dummy资源的作用
  dummy_sampler_info_.imageLayout = dummy_target_info_.imageLayout;
  dummy_sampler_info_.imageView = dummy_image_view_;
  dummy_target_info_.imageView = dummy_image_view_;

  // 没有采样器时，需要创建一个
  if (dummy_sampler_info_.sampler == VK_NULL_HANDLE) {
    VkSamplerCreateInfo sampler_info {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 1.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };
    vkCreateSampler(device_, &sampler_info, nullptr, &dummy_sampler_info_.sampler);
  }

  // 因为是新的DescriptorSet，所以需要重新绑定该DescriptorSet相关的资源
  VkDescriptorBufferInfo descriptor_buffers[UBUFFER_BINGDING_COUNT];
  VkDescriptorImageInfo descriptor_samplers[SAMPLER_BINDING_COUNT];
  VkDescriptorImageInfo descriptor_input_attachments[TARGET_BINDING_COUNT];

  VkWriteDescriptorSet descriptor_writers[UBUFFER_BINGDING_COUNT + SAMPLER_BINDING_COUNT + TARGET_BINDING_COUNT];

  uint32_t n_write = 0;
  VkWriteDescriptorSet* writers = descriptor_writers;

  // 重新绑定uniform buffer的writer
  for (uint32_t binding = 0; binding < UBUFFER_BINGDING_COUNT; ++binding) {
    VkWriteDescriptorSet& writer_info = writers[n_write++];
    // 如果已经绑定，则使用现有的
    if (current_descriptor_.uniform_buffers[binding]) {
      VkDescriptorBufferInfo& buffer_info = descriptor_buffers[binding];
      // 将现有的信息拷贝到descriptor buffers
      buffer_info.buffer = current_descriptor_.uniform_buffers[binding];
      buffer_info.offset = current_descriptor_.uniform_buffer_offsets[binding];
      buffer_info.range = current_descriptor_.uniform_buffer_sizes[binding];

      writer_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writer_info.pNext = nullptr;
      writer_info.dstArrayElement = 0;
      writer_info.descriptorCount = 1;
      writer_info.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      writer_info.pImageInfo = nullptr;
      writer_info.pBufferInfo = &buffer_info;
      writer_info.pTexelBufferView = nullptr;
    } else {
      //没有则使用dummy的默认属性
      writer_info = dummy_buffer_write_info_;
    }
    // descriptor_bundle即当前的资源
    // handles[0]即uniform buffer对应的 descriptor
    writer_info.dstSet = descriptor_bundle->handles[0];
    // 即绑定槽的索引
    writer_info.dstBinding = binding;
  }

  // 绑定sampler的writer
  for (uint32_t binding = 0; binding < SAMPLER_BINDING_COUNT; ++binding) {
    VkWriteDescriptorSet& writer_info = writers[n_write++];
    if (current_descriptor_.samplers[binding].sampler) {
      // 已经存在则直接使用
      VkDescriptorImageInfo& sampler_info = descriptor_samplers[binding];
      sampler_info = current_descriptor_.samplers[binding];

      writer_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writer_info.pNext = nullptr;
      writer_info.dstArrayElement = 0;
      writer_info.descriptorCount = 1;
      writer_info.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      writer_info.pImageInfo = &sampler_info;
      writer_info.pBufferInfo = nullptr;
      writer_info.pTexelBufferView = nullptr;
    } else {
      //没有则使用dummy的默认属性
      writer_info = dummy_sampler_write_info_;
    }
    // 1为sampler
    writer_info.dstSet = descriptor_bundle->handles[1];
    writer_info.dstBinding = binding;
  }

  // 绑定target的writer
  for (uint32_t binding = 0; binding < TARGET_BINDING_COUNT; ++binding) {
    VkWriteDescriptorSet& writer_info = writers[n_write++];
    if (current_descriptor_.input_attachments[binding].sampler) {
      // 已经存在则直接使用
      VkDescriptorImageInfo& target_info = descriptor_input_attachments[binding];
      target_info = current_descriptor_.input_attachments[binding];

      writer_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writer_info.pNext = nullptr;
      writer_info.dstArrayElement = 0;
      writer_info.descriptorCount = 1;
      writer_info.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      writer_info.pImageInfo = &target_info;
      writer_info.pBufferInfo = nullptr;
      writer_info.pTexelBufferView = nullptr;
    } else {
      //没有则使用dummy的默认属性
      writer_info = dummy_target_write_info_;
    }
    // 2为render target
    writer_info.dstSet = descriptor_bundle->handles[2];
    writer_info.dstBinding = binding;
  }
  // 更新对应的DescriptorSet
  vkUpdateDescriptorSets(device_, n_write, writers, 0, nullptr);
  // 通知外部重新绑定set
  *bind = true;
  return true;
}
/**
 *
 * */
bool VulkanPipelineCache::GetOrCreatePipeline(VkPipeline *pipeline, bool* bind) noexcept {
  *bind = false;
  // 取当前pipeline的值引用，方便后面操作
  PipelineVal*& current_pipeline_val = cmd_buffer_state_[current_cmd_buffer_].current_pipeline;

  //同样，先检查有无改变，无改变直接返回
  if (!VulkanUtils::GetBit(dirty_pipeline_, current_cmd_buffer_)) {
    assert(current_pipeline_val);
    current_pipeline_val->age = 0;
    *pipeline = current_pipeline_val->handle;
    return true;
  }

  // 改变了，先检查cache，有则直接返回
  auto iter = pipelines_.find(current_pipeline_);
  if (iter != pipelines_.end()) {
    current_pipeline_val = &iter->second;
    // 先将cache中的age设为0
    current_pipeline_val->age = 0;
    *pipeline = current_pipeline_val->handle;
    VulkanUtils::SetBit(&dirty_pipeline_, current_cmd_buffer_, 0);
    *bind = true;
    return true;
  }

  // cache中没有，需要重新创建
  VkPipelineShaderStageCreateInfo shader_stage[SHADER_MODULE_COUNT];
  shader_stage[0] = VkPipelineShaderStageCreateInfo{};
  shader_stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shader_stage[0].pName = "main";

  shader_stage[1] = VkPipelineShaderStageCreateInfo{};
  shader_stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage[1].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shader_stage[1].pName = "main";

  // 设置混合状态
  VkPipelineColorBlendAttachmentState color_blend_attachments[MAX_SUPPORTED_RENDER_TARGET_COUNT];
  VkPipelineColorBlendStateCreateInfo color_blend_state = {};
  color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  //todo:解释
  color_blend_state.attachmentCount = 1;
  color_blend_state.pAttachments = color_blend_attachments;


}
}  // namespace our_graph