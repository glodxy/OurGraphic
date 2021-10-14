//
// Created by Glodxy on 2021/9/26.
//

#include "VulkanPipelineCache.h"
#include "VulkanContext.h"

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
  assert(device != VK_NULL_HANDLE);

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
  overflow = !GetOrCreateDescriptors(descriptor_set, &bind);

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
  descriptors_.try_emplace(current_descriptor_, DescriptorBundle {});

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
  VkDescriptorBufferInfo descriptor_buffers[UBUFFER_BINDING_COUNT];
  VkDescriptorImageInfo descriptor_samplers[SAMPLER_BINDING_COUNT];
  VkDescriptorImageInfo descriptor_input_attachments[TARGET_BINDING_COUNT];

  VkWriteDescriptorSet descriptor_writers[UBUFFER_BINDING_COUNT + SAMPLER_BINDING_COUNT + TARGET_BINDING_COUNT];

  uint32_t n_write = 0;
  VkWriteDescriptorSet* writers = descriptor_writers;

  // 重新绑定uniform buffer的writer
  for (uint32_t binding = 0; binding < UBUFFER_BINDING_COUNT; ++binding) {
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
    if (current_descriptor_.input_attachments[binding].imageView != VK_NULL_HANDLE) {
      // 已经存在则直接使用
      VkDescriptorImageInfo& target_info = descriptor_input_attachments[binding];
      target_info = current_descriptor_.input_attachments[binding];

      writer_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writer_info.pNext = nullptr;
      writer_info.dstArrayElement = 0;
      writer_info.descriptorCount = 1;
      writer_info.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
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
  shader_stage[0].module = current_pipeline_.shaders[0];

  shader_stage[1] = VkPipelineShaderStageCreateInfo{};
  shader_stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shader_stage[1].pName = "main";
  shader_stage[1].module = current_pipeline_.shaders[1];

  // 设置混合状态
  VkPipelineColorBlendAttachmentState color_blend_attachments[MAX_SUPPORTED_RENDER_TARGET_COUNT];
  VkPipelineColorBlendStateCreateInfo color_blend_state = {};
  color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  //todo:解释
  color_blend_state.attachmentCount = 1;
  color_blend_state.pAttachments = color_blend_attachments;

  // 标识相应的数量
  uint32_t num_vertex_attribs = 0;
  uint32_t num_vertex_buffers = 0;
  // 查看设置了几个属性
  for (int i = 0; i < VERTEX_ATTRIBUTE_COUNT; ++i) {
    if (current_pipeline_.vertex_attributes[i].format > 0) {
      num_vertex_attribs++;
    }
    if (current_pipeline_.vertex_buffer->stride > 0) {
      num_vertex_buffers++;
    }
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
  vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_state.vertexBindingDescriptionCount = num_vertex_buffers;
  vertex_input_state.pVertexBindingDescriptions = current_pipeline_.vertex_buffer;
  vertex_input_state.vertexAttributeDescriptionCount = num_vertex_attribs;
  vertex_input_state.pVertexAttributeDescriptions = current_pipeline_.vertex_attributes;

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
  input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_state.topology = current_pipeline_.topology;

  VkPipelineViewportStateCreateInfo viewport_state = {};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  VkDynamicState dynamic_state_enables[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo dynamic_state = {};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.pDynamicStates = dynamic_state_enables;
  dynamic_state.dynamicStateCount = 2;

  const bool has_fragment_shader = shader_stage[1].module != VK_NULL_HANDLE;

  VkGraphicsPipelineCreateInfo pipeline_create_info = {};
  pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_create_info.layout = pipeline_layout_;
  pipeline_create_info.renderPass = current_pipeline_.render_pass;
  pipeline_create_info.subpass = current_pipeline_.subpass_index;
  pipeline_create_info.stageCount = has_fragment_shader ? SHADER_MODULE_COUNT : 1;
  pipeline_create_info.pStages = shader_stage;
  pipeline_create_info.pVertexInputState = &vertex_input_state;
  pipeline_create_info.pInputAssemblyState = &input_assembly_state;

  VkPipelineRasterizationStateCreateInfo vk_raster = {};
  vk_raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  pipeline_create_info.pRasterizationState = &vk_raster;

  VkPipelineMultisampleStateCreateInfo vk_ms = {};
  vk_ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  pipeline_create_info.pMultisampleState = &vk_ms;

  //todo:解释
  VkPipelineDepthStencilStateCreateInfo vk_ds = {};
  vk_ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  vk_ds.front = vk_ds.back = {
      .failOp = VK_STENCIL_OP_KEEP,
      .passOp = VK_STENCIL_OP_KEEP,
      .depthFailOp = VK_STENCIL_OP_KEEP,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .compareMask = 0u,
      .writeMask = 0u,
      .reference = 0u,
  };
  pipeline_create_info.pDepthStencilState = &vk_ds;

  const auto& raster = current_pipeline_.raster_state;

  vk_raster.depthClampEnable = raster.rasterization.depth_clamp_enable;
  vk_raster.rasterizerDiscardEnable = raster.rasterization.rasterizer_discard_enable;
  vk_raster.polygonMode = raster.rasterization.polygon_mode;
  vk_raster.cullMode = raster.rasterization.cull_mode;
  vk_raster.frontFace = raster.rasterization.front_face;
  vk_raster.depthBiasEnable = raster.rasterization.depth_bias_enable;
  vk_raster.depthBiasConstantFactor = raster.rasterization.depth_bias_constant_factor;
  vk_raster.depthBiasClamp = raster.rasterization.depth_bias_clamp;
  vk_raster.depthBiasSlopeFactor = raster.rasterization.depth_bias_slope_factor;
  vk_raster.lineWidth = raster.rasterization.line_width;

  vk_ms.rasterizationSamples = raster.multisampling.rasterization_samples;
  vk_ms.sampleShadingEnable = raster.multisampling.sample_shading_enable;
  vk_ms.minSampleShading = raster.multisampling.min_sample_shading;
  vk_ms.alphaToCoverageEnable = raster.multisampling.alpha_to_coverage_enable;
  vk_ms.alphaToOneEnable = raster.multisampling.alpha_to_one_enable;

  vk_ds.depthTestEnable = raster.depth_stencil.depth_test_enable;
  vk_ds.depthWriteEnable = raster.depth_stencil.depth_write_enable;
  vk_ds.depthCompareOp = raster.depth_stencil.depth_compare_op;
  vk_ds.depthBoundsTestEnable = raster.depth_stencil.depth_bounds_test_enable;
  vk_ds.stencilTestEnable = raster.depth_stencil.stencil_test_enable;
  vk_ds.minDepthBounds = raster.depth_stencil.min_depth_bounds;
  vk_ds.maxDepthBounds = raster.depth_stencil.max_depth_bounds;

  pipeline_create_info.pColorBlendState = &color_blend_state;
  pipeline_create_info.pViewportState = &viewport_state;
  pipeline_create_info.pDynamicState = &dynamic_state;

  // 没有片段着色器时不需要设置颜色混合
  if (has_fragment_shader) {
    // 设置所有的color attachment的渲染状态
    // 都设置为raster中设置的状态
    // 即所有color attachment的渲染状态相同
    // todo:支持不同的blend
    color_blend_state.attachmentCount = current_pipeline_.raster_state.color_target_count;
    for (auto &target: color_blend_attachments) {
      target = current_pipeline_.raster_state.blending;
    }
  } else {
    color_blend_state.attachmentCount = 0;
  }

  LOG_INFO("VulkanPipeline", "CreateGraphicPipeline with shaders:({}, {})",
           (void*)shader_stage[0].module, (void*)shader_stage[1].module);

  VkResult result = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1,
                                              &pipeline_create_info, nullptr,
                                              pipeline);
  if (result != VK_SUCCESS) {
    LOG_ERROR("VulkanPipeline", "CreateGraphicPipeline Failed! code:{}", result);
    return false;
  }

  const PipelineVal cache_val = {*pipeline, 0u};
  current_pipeline_val = &(pipelines_.try_emplace(current_pipeline_, cache_val).first->second);
  VulkanUtils::SetBit(&dirty_pipeline_, current_cmd_buffer_, 0);
  *bind = true;

  return true;
}
/**
 * 绑定着色器
 * */
void VulkanPipelineCache::BindProgramBundle(const ProgramBundle &bundle) noexcept {
  const VkShaderModule shaders[2] = {bundle.vertex, bundle.fragment};
  for (int i = 0; i < SHADER_MODULE_COUNT; ++i) {
    if (current_pipeline_.shaders[i] != shaders[i]) {
      MarkDirtyPipeline();
      current_pipeline_.shaders[i] = shaders[i];
    }
  }
}

/**
 * 绑定光栅化状态
 * */
void VulkanPipelineCache::BindRasterState(const RasterState &raster_state) noexcept {
  auto& raster_dst = current_pipeline_.raster_state.rasterization;
  auto& raster_src = raster_state.rasterization;

  auto& blend_dst = current_pipeline_.raster_state.blending;
  auto& blend_src = raster_state.blending;

  auto& ds_dst = current_pipeline_.raster_state.depth_stencil;
  auto& ds_src = raster_state.depth_stencil;

  auto& ms_dst = current_pipeline_.raster_state.multisampling;
  auto& ms_src = raster_state.multisampling;

  if (raster_dst.depth_clamp_enable != raster_src.depth_clamp_enable ||
      raster_dst.rasterizer_discard_enable != raster_src.rasterizer_discard_enable ||
      raster_dst.polygon_mode != raster_src.polygon_mode ||
      raster_dst.cull_mode != raster_src.cull_mode ||
      raster_dst.front_face != raster_src.front_face ||
      raster_dst.depth_bias_enable != raster_src.depth_bias_enable ||
      raster_dst.depth_bias_constant_factor != raster_src.depth_bias_constant_factor ||
      raster_dst.depth_bias_clamp != raster_src.depth_bias_clamp ||
      raster_dst.depth_bias_slope_factor != raster_src.depth_bias_slope_factor ||
      raster_dst.line_width != raster_src.line_width ||

      blend_dst.blendEnable != blend_src.blendEnable ||
      blend_dst.srcColorBlendFactor != blend_src.srcColorBlendFactor ||
      blend_dst.dstColorBlendFactor != blend_src.dstColorBlendFactor ||
      blend_dst.colorBlendOp != blend_src.colorBlendOp ||
      blend_dst.srcAlphaBlendFactor != blend_src.srcAlphaBlendFactor ||
      blend_dst.dstAlphaBlendFactor != blend_src.dstAlphaBlendFactor ||
      blend_dst.alphaBlendOp != blend_src.alphaBlendOp ||
      blend_dst.colorWriteMask != blend_src.colorWriteMask ||

      ds_dst.depth_test_enable != ds_src.depth_test_enable ||
      ds_dst.depth_write_enable != ds_src.depth_write_enable ||
      ds_dst.depth_compare_op != ds_src.depth_compare_op ||
      ds_dst.depth_bounds_test_enable != ds_src.depth_bounds_test_enable ||
      ds_dst.stencil_test_enable != ds_src.stencil_test_enable ||
      ds_dst.min_depth_bounds != ds_src.min_depth_bounds ||
      ds_dst.max_depth_bounds != ds_src.max_depth_bounds ||

      ms_dst.rasterization_samples != ms_src.rasterization_samples ||
      ms_dst.sample_shading_enable != ms_src.sample_shading_enable ||
      ms_dst.min_sample_shading != ms_src.min_sample_shading ||
      ms_dst.alpha_to_coverage_enable != ms_src.alpha_to_coverage_enable ||
      ms_dst.alpha_to_one_enable != ms_src.alpha_to_one_enable ||

      current_pipeline_.raster_state.color_target_count != raster_state.color_target_count) {
    MarkDirtyPipeline();
    current_pipeline_.raster_state = raster_state;
  }
}

void VulkanPipelineCache::BindRenderPass(VkRenderPass render_pass, int subpass_index) noexcept {
  if (current_pipeline_.render_pass != render_pass ||
      current_pipeline_.subpass_index != subpass_index) {
    MarkDirtyPipeline();
    current_pipeline_.render_pass = render_pass;
    current_pipeline_.subpass_index = subpass_index;
  }
}

void VulkanPipelineCache::BindPrimitiveTopology(VkPrimitiveTopology topology) noexcept {
  if (current_pipeline_.topology != topology) {
    MarkDirtyPipeline();
    current_pipeline_.topology = topology;
  }
}

void VulkanPipelineCache::BindVertexArray(const VertexArray &array) noexcept {
  for (int i = 0; i < VERTEX_ATTRIBUTE_COUNT; ++i) {
    auto& attrib_dst = current_pipeline_.vertex_attributes[i];
    const auto& attrib_src = array.attribution[i];
    if (attrib_dst.location != attrib_src.location ||
        attrib_dst.binding != attrib_src.binding ||
        attrib_dst.format != attrib_src.format ||
        attrib_dst.offset != attrib_src.offset) {
      attrib_dst.format = attrib_src.format;
      attrib_dst.binding = attrib_src.binding;
      attrib_dst.location = attrib_src.location;
      attrib_dst.offset = attrib_src.offset;
      MarkDirtyPipeline();
    }
    VkVertexInputBindingDescription& buffer_dst = current_pipeline_.vertex_buffer[i];
    const VkVertexInputBindingDescription& buffer_src = array.buffers[i];
    if (buffer_dst.binding != buffer_src.binding ||
        buffer_dst.stride != buffer_src.stride) {
      buffer_dst.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      buffer_dst.binding = buffer_src.binding;
      buffer_dst.stride = buffer_src.stride;
      MarkDirtyPipeline();
    }
  }
}

void VulkanPipelineCache::BindUniformBuffer(uint32_t binding_index,
                                            VkBuffer uniform_buffer,
                                            VkDeviceSize offset,
                                            VkDeviceSize size) noexcept {
  assert(binding_index < UBUFFER_BINDING_COUNT);
  auto& key = current_descriptor_;
  if (key.uniform_buffers[binding_index] != uniform_buffer ||
      key.uniform_buffer_offsets[binding_index] != offset ||
      key.uniform_buffer_sizes[binding_index] != size) {
    key.uniform_buffers[binding_index] = uniform_buffer;
    key.uniform_buffer_offsets[binding_index] = offset;
    key.uniform_buffer_sizes[binding_index] = size;
    MarkDirtyDescriptor();
  }
}

void VulkanPipelineCache::BindSamplers(VkDescriptorImageInfo *samplers) noexcept {
  for (int i = 0; i < SAMPLER_BINDING_COUNT; ++i) {
    const VkDescriptorImageInfo& sampler_src = samplers[i];
    VkDescriptorImageInfo& sampler_dst = current_descriptor_.samplers[i];
    if (sampler_dst.sampler != sampler_src.sampler ||
        sampler_dst.imageView != sampler_src.imageView ||
        sampler_dst.imageLayout != sampler_src.imageLayout) {
      sampler_dst = sampler_src;
      MarkDirtyDescriptor();
    }
  }
}

void VulkanPipelineCache::BindInputAttachment(uint32_t binding_index, VkDescriptorImageInfo image_info) noexcept {
  assert(binding_index < TARGET_BINDING_COUNT);
  VkDescriptorImageInfo& target_dst = current_descriptor_.input_attachments[binding_index];
  if (image_info.imageView != target_dst.imageView ||
      image_info.imageLayout != target_dst.imageLayout) {
    target_dst = image_info;
    MarkDirtyDescriptor();
  }
}

VulkanPipelineCache::UniformBufferBinding VulkanPipelineCache::GetUniformBufferBinging(
    uint32_t binding_index) noexcept {
  auto& key = current_descriptor_;
  return {
    key.uniform_buffers[binding_index],
    key.uniform_buffer_offsets[binding_index],
    key.uniform_buffer_sizes[binding_index],
  };
}

void VulkanPipelineCache::UnBindUniformBuffer(VkBuffer uniform_buffer) noexcept {
  auto& key = current_descriptor_;
  for (int i = 0; i < UBUFFER_BINDING_COUNT; ++i) {
    if (key.uniform_buffers[i] == uniform_buffer) {
      key.uniform_buffers[i] = {};
      key.uniform_buffer_sizes[i] = {};
      key.uniform_buffer_offsets[i] = {};
      MarkDirtyDescriptor();
    }
  }
}

void VulkanPipelineCache::UnBindImageView(VkImageView image_view) noexcept {
  for (auto& sampler : current_descriptor_.samplers) {
    if (sampler.imageView == image_view) {
      sampler = {};
      MarkDirtyDescriptor();
    }
  }

  for (auto& target : current_descriptor_.input_attachments) {
    if (target.imageView == image_view) {
      target = {};
      MarkDirtyDescriptor();
    }
  }
}

void VulkanPipelineCache::DestroyAllCache() noexcept {
  DestroyLayoutsAndDescriptors();

  for (auto& iter : pipelines_) {
    vkDestroyPipeline(device_, iter.second.handle, nullptr);
  }
  pipelines_.clear();
  // 清除所有cmd buffer所持有的pipeline
  for (int i = 0; i < MAX_COMMAND_BUFFERS_COUNT; ++i) {
    cmd_buffer_state_[i].current_pipeline = nullptr;
  }
  MarkDirtyPipeline();

  if (dummy_sampler_info_.sampler) {
    vkDestroySampler(device_, dummy_sampler_info_.sampler, nullptr);
    dummy_sampler_info_.sampler = VK_NULL_HANDLE;
  }

  vmaDestroyBuffer(allocator_, dummy_buffer_, dummy_memory_);
  dummy_buffer_ = VK_NULL_HANDLE;
  dummy_memory_ = VK_NULL_HANDLE;
}

/**
 * 该函数在command buffer发生变更时被调用
 * */
void VulkanPipelineCache::OnCommandBuffer(const VulkanCommandBuffer &cmd_buffer) {
  // 设置当前的cmd buffer的序号
  current_cmd_buffer_ = cmd_buffer.index;

  // 标识该位发生了改变，需重新设置当前的pipeline和descriptor
  VulkanUtils::SetBit(&dirty_pipeline_, current_cmd_buffer_);
  VulkanUtils::SetBit(&dirty_descriptor_, current_cmd_buffer_);
  cmd_buffer_state_[current_cmd_buffer_].scissor = {};

  /**
   * 检查cache中所有的descriptors
   * 如果该descriptor未绑定至任何的cmd buffer，则从cache中移除，
   * 并将该DescriptorSets移至缓冲区以方便之后使用
   * */
  for (auto iter = descriptors_.begin(); iter != descriptors_.end();) {
    const auto& value = iter->second;
    if (value.command_buffers == 0) {
      descriptor_sets_[0].push_back(value.handles[0]);
      descriptor_sets_[1].push_back(value.handles[1]);
      descriptor_sets_[2].push_back(value.handles[2]);
      iter = descriptors_.erase(iter);
    } else {
      ++iter;
    }
  }

  /**
   * 将cache中所有pipeline的等待周期数+1
   * */
  for (auto& pipeline : pipelines_) {
    pipeline.second.age++;
  }

  /**
   * 清除所有等待数超过限制（即长期未使用）的pipeline
   * */
  for (auto iter = pipelines_.begin(); iter != pipelines_.end();) {
     if (iter->second.age > MAX_PIPELINE_AGE) {
       vkDestroyPipeline(device_, iter->second.handle, nullptr);
       iter = pipelines_.erase(iter);
     } else {
       ++iter;
     }
  }

   /**
    * 对所有剩下的DescriptorSets（即有绑定至cmd buffer）
    * 消除其在current cmd buffer上的脏位，标识其没被该cmd buffer使用
    * */
  for (auto& iter : descriptors_) {
    VulkanUtils::SetBit(&iter.second.command_buffers, current_cmd_buffer_, 0);
  }

  bool can_clear_extinct_pool = true;
  for (auto& bundle : extinct_descriptor_bundles_) {
    VulkanUtils::SetBit(&bundle.command_buffers, current_cmd_buffer_, 0);
    if (bundle.command_buffers != 0) {
      can_clear_extinct_pool = false;
      break;
    }
  }

    // 只有extinct bundles中的所有descriptor都不再被cmd buffer使用才移除
  if (can_clear_extinct_pool) {
    for(auto pool : extinct_descriptor_pools_) {
      vkDestroyDescriptorPool(device_, pool, nullptr);
    }
    extinct_descriptor_pools_.clear();
    extinct_descriptor_bundles_.clear();
  }
}

void VulkanPipelineCache::CreateLayoutsAndDescriptors() noexcept {
  // 该变量为临时变量，仅用于给其他元素赋值
  VkDescriptorSetLayoutBinding binding = {};
  binding.descriptorCount = 1; // 每一类只使用一个descriptor
  binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

  VkDescriptorSetLayoutBinding ubuffer_binding[UBUFFER_BINDING_COUNT];
  binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  for (int i = 0; i < UBUFFER_BINDING_COUNT; ++i) {
    binding.binding = i;
    ubuffer_binding[i] = binding;
  }
  // 创建ubuffer的布局
  VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {};
  descriptor_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_layout_info.bindingCount = UBUFFER_BINDING_COUNT;
  descriptor_layout_info.pBindings = ubuffer_binding;
  vkCreateDescriptorSetLayout(device_, &descriptor_layout_info, nullptr, &descriptor_set_layouts_[0]);

  VkDescriptorSetLayoutBinding sampler_binding[SAMPLER_BINDING_COUNT];
  binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  for (int i = 0; i < SAMPLER_BINDING_COUNT; ++i) {
    binding.binding = i;
    sampler_binding[i] = binding;
  }
  // 创建sampler的布局
  descriptor_layout_info.bindingCount = SAMPLER_BINDING_COUNT;
  descriptor_layout_info.pBindings = sampler_binding;
  vkCreateDescriptorSetLayout(device_, &descriptor_layout_info, nullptr, &descriptor_set_layouts_[1]);

  VkDescriptorSetLayoutBinding target_binding[TARGET_BINDING_COUNT];
  binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
  binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  for (int i = 0; i < TARGET_BINDING_COUNT; ++i) {
    binding.binding = i;
    target_binding[i] = binding;
  }
  // 创建target的布局
  descriptor_layout_info.bindingCount = TARGET_BINDING_COUNT;
  descriptor_layout_info.pBindings = target_binding;
  vkCreateDescriptorSetLayout(device_, &descriptor_layout_info, nullptr, &descriptor_set_layouts_[2]);

  // 创建pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
  pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_create_info.setLayoutCount = 3;
  pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts_;
  VkResult res = vkCreatePipelineLayout(device_, &pipeline_layout_create_info, nullptr, &pipeline_layout_);
  if (res != VK_SUCCESS) {
    LOG_ERROR("VulkanPipelineCache", "CreatePipelineLayout Failed! code:{}", res);
  }

  descriptor_pool_ = CreateDescriptorPool(descriptor_pool_size_);
}

VkDescriptorPool VulkanPipelineCache::CreateDescriptorPool(uint32_t size) const {
  VkDescriptorPoolSize pool_sizes[DESCRIPTOR_TYPE_COUNT] = {};
  VkDescriptorPoolCreateInfo pool_info {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
    .maxSets = size * DESCRIPTOR_TYPE_COUNT,
    .poolSizeCount = DESCRIPTOR_TYPE_COUNT,
    .pPoolSizes = pool_sizes
  };
  pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //maxSets为pool的sets数量，而对于ubuffer， 每个sets有固定的数量
  pool_sizes[0].descriptorCount = pool_info.maxSets * UBUFFER_BINDING_COUNT;
  pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_sizes[1].descriptorCount = pool_info.maxSets * SAMPLER_BINDING_COUNT;
  pool_sizes[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
  pool_sizes[2].descriptorCount = pool_info.maxSets * TARGET_BINDING_COUNT;

  VkDescriptorPool pool;
  VkResult res = vkCreateDescriptorPool(device_, &pool_info, nullptr, &pool);
  if (res != VK_SUCCESS) {
    LOG_ERROR("VulkanPipelineCache", "CreateDescriptorPool Failed! code:{}", res);
  }
  return pool;
}

void VulkanPipelineCache::DestroyLayoutsAndDescriptors() noexcept {
  // 没有layout则无需销毁
  if (pipeline_layout_ == VK_NULL_HANDLE) {
    return;
  }

  LOG_INFO("VulkanPipelineCache", "Destroying {} bundles of desc sets",
           descriptors_.size());

  // 清除缓冲区
  for (auto& sets : descriptor_sets_) {
    sets.clear();
  }

  descriptors_.clear();

  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
  pipeline_layout_ = VK_NULL_HANDLE;
  for (int i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
    vkDestroyDescriptorSetLayout(device_, descriptor_set_layouts_[i], nullptr);
    descriptor_set_layouts_[i] = {};
  }
  vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
  descriptor_pool_ = VK_NULL_HANDLE;

  for (int i = 0; i < MAX_COMMAND_BUFFERS_COUNT; ++i) {
    cmd_buffer_state_[i].current_descriptor_bundle = nullptr;
  }

  // 清除所有待清除的pool
  for (auto pool : extinct_descriptor_pools_) {
    vkDestroyDescriptorPool(device_, pool, nullptr);
  }

  extinct_descriptor_pools_.clear();
  extinct_descriptor_bundles_.clear();
  MarkDirtyDescriptor();
}

void VulkanPipelineCache::GrowDescriptorPool() noexcept {
  // 首先将当前使用的池标记为待删除
  extinct_descriptor_pools_.push_back(descriptor_pool_);

  // 容量翻倍
  descriptor_pool_size_ *= 2;
  descriptor_pool_ = CreateDescriptorPool(descriptor_pool_size_);

  // 清除当前的缓冲区（因为池发生了变化，所以需重新分配）
  for (auto& sets : descriptor_sets_) {
    sets.clear();
  }

  // 清除当前cache
  for (auto iter : descriptors_) {
    extinct_descriptor_bundles_.push_back(iter.second);
  }

  descriptors_.clear();
}

bool operator==(const our_graph::VulkanPipelineCache::RasterState &s1,
                                                       const our_graph::VulkanPipelineCache::RasterState &s2) {
  return 0 == memcmp((void*)&s1, (void*)&s2, sizeof(RasterState));
}

bool VulkanPipelineCache::PipelineEqual::operator()(VulkanPipelineCache::PipelineKey k1,
                                                    VulkanPipelineCache::PipelineKey k2) const {
  for (int i = 0; i < SHADER_MODULE_COUNT; ++i) {
    if (k1.shaders[i] != k2.shaders[i]) {
      return false;
    }
  }
  for (int i = 0; i < MAX_VERTEX_ATTRIBUTE_COUNT; ++i) {
    if (k1.vertex_attributes[i].format != k2.vertex_attributes[i].format ||
        k1.vertex_attributes[i].offset != k2.vertex_attributes[i].offset ||
        k1.vertex_attributes[i].binding != k2.vertex_attributes[i].binding ||
        k1.vertex_attributes[i].location != k2.vertex_attributes[i].location) {
      return false;
    }
  }
  for (int i = 0; i < MAX_VERTEX_ATTRIBUTE_COUNT; ++i) {
    if (k1.vertex_buffer[i].stride!= k2.vertex_buffer[i].stride ||
        k1.vertex_buffer[i].binding!= k2.vertex_buffer[i].binding ||
        k1.vertex_buffer[i].inputRate!= k2.vertex_buffer[i].inputRate) {
      return false;
    }
  }
  return k1.render_pass == k2.render_pass &&
         k1.topology == k2.topology &&
         k1.subpass_index == k2.subpass_index &&
         k1.raster_state == k2.raster_state;
}

bool VulkanPipelineCache::DescEqual::operator()(const VulkanPipelineCache::DescriptorKey& k1,
                                                const VulkanPipelineCache::DescriptorKey& k2) const {
  for (uint32_t i = 0; i < UBUFFER_BINDING_COUNT; i++) {
    if (k1.uniform_buffers[i] != k2.uniform_buffers[i] ||
        k1.uniform_buffer_offsets[i] != k2.uniform_buffer_offsets[i] ||
        k1.uniform_buffer_sizes[i] != k2.uniform_buffer_sizes[i]) {
      return false;
    }
  }
  for (uint32_t i = 0; i < SAMPLER_BINDING_COUNT; i++) {
    if (k1.samplers[i].sampler != k2.samplers[i].sampler ||
        k1.samplers[i].imageView != k2.samplers[i].imageView ||
        k1.samplers[i].imageLayout != k2.samplers[i].imageLayout) {
      return false;
    }
  }
  for (uint32_t i = 0; i < TARGET_BINDING_COUNT; i++) {
    if (k1.input_attachments[i].imageView != k2.input_attachments[i].imageView ||
        k1.input_attachments[i].imageLayout != k2.input_attachments[i].imageLayout) {
      return false;
    }
  }
  return true;
}

std::size_t VulkanPipelineCache::PipelineHash::operator()(const PipelineKey &key) const {
  const char* begin = (const char*) &key;
  std::string addr = std::string(begin, begin + sizeof(PipelineKey));
  std::hash<std::string> hasher;
  return hasher(addr);
}

std::size_t VulkanPipelineCache::DescHash::operator()(const DescriptorKey &key) const {
  const char* begin = (const char*) &key;
  std::string addr = std::string(begin, begin + sizeof(DescriptorKey));
  std::hash<std::string> hasher;
  return hasher(addr);
}

}  // namespace our_graph