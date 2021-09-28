//
// Created by Glodxy on 2021/9/24.
//

#include "VulkanFBOCache.h"

#include "../include_internal/Constants.h"

namespace our_graph {

bool VulkanFBOCache::RenderPassEq::operator()(const RenderPassKey& k1,
                                              const RenderPassKey& k2) const {
  if (k1.clear != k2.clear) return false;
  if (k1.discard_start != k2.discard_start) return false;
  if (k1.discard_end != k2.discard_end) return false;
  if (k1.samples != k2.samples) return false;
  if (k1.needs_resolve_mask != k2.needs_resolve_mask) return false;
  if (k1.subpass_mask != k2.subpass_mask) return false;
  if (k1.depth_layout != k2.depth_layout) return false;
  if (k1.depth_format != k2.depth_format) return false;
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
    if (k1.color_layout[i] != k2.color_layout[i]) return false;
    if (k1.color_format[i] != k2.color_format[i]) return false;
  }
  return true;
}

bool VulkanFBOCache::FBOKeyEq::operator()(const FBOKey& k1, const FBOKey& k2) const {
  if (k1.render_pass != k2.render_pass) return false;
  if (k1.width != k2.width) return false;
  if (k1.height != k2.height) return false;
  if (k1.layers != k2.layers) return false;
  if (k1.samples != k2.samples) return false;
  if (k1.depth != k2.depth) return false;
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
    if (k1.color[i] != k2.color[i]) return false;
    if (k1.resolve[i] != k2.resolve[i]) return false;
  }
  return true;
}

VulkanFBOCache::VulkanFBOCache() {}

VulkanFBOCache::~VulkanFBOCache() {

}

VkFramebuffer VulkanFBOCache::GetFrameBuffer(FBOKey config) noexcept {
  auto framebuffer_iter = frame_buffer_cache_.find(config);
  // 如果存在则更新使用的时间戳，并直接返回
  if ((framebuffer_iter != frame_buffer_cache_.end()) &&
      (framebuffer_iter->second.handle != VK_NULL_HANDLE)) {
    framebuffer_iter->second.timestamp = current_time;
    return framebuffer_iter->second.handle;
  }

  VkImageView attachments[MAX_SUPPORTED_RENDER_TARGET_COUNT + MAX_SUPPORTED_RENDER_TARGET_COUNT + 1];
  uint32_t attachment_count = 0;
  // 将颜色缓存写入
  for (auto attachment : config.color) {
    if (attachment) {
      attachments[attachment_count++] = attachment;
    }
  }
  // 将解析用的缓存写入
  for (auto attachment : config.resolve) {
    if (attachment) {
      attachments[attachment_count++] = attachment;
    }
  }
  if (config.depth) {
    attachments[attachment_count++] = config.depth;
  }

  LOG_INFO("VulkanFBOCache", "Create FrameBuffer {}X{}, \n"
                             "Render Pass:{}, \n"
                             "Samples:{}, \n"
                             "Depth:{}, \n"
                             "AttachmentCount:{}",
                             config.width, config.height,
                             (void*)config.render_pass,
                             config.samples,
                             config.depth? 1 : 0,
                             attachment_count);

  // 不存在时，则创建一个buffer
  VkFramebufferCreateInfo create_info {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .renderPass = config.render_pass,
    .attachmentCount = attachment_count,
    .pAttachments = attachments,
    .width = config.width,
    .height = config.height,
    .layers = config.layers,
  };
  renderpass_ref_cnt_[create_info.renderPass]++;
  VkFramebuffer framebuffer;

  VkDevice* device = VulkanContext::Get().device_;
  VkResult result = vkCreateFramebuffer(*device, &create_info, nullptr, &framebuffer);
  if (result != VK_SUCCESS) {
    LOG_ERROR("VulkanFBOCache", "CreateFrameBuffer Failed!code:{}",result);
  }
  frame_buffer_cache_[config] = {framebuffer, current_time};
  return framebuffer;
}

VkRenderPass VulkanFBOCache::GetRenderPass(RenderPassKey config) noexcept {
  auto iter = render_pass_cache_.find(config);
  if ((iter != render_pass_cache_.end()) &&
      (iter->second.handle != VK_NULL_HANDLE)) {
    iter->second.timestamp = current_time;
    return iter->second.handle;
  }
  const bool is_swapchain =
      config.color_layout[0] == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  const bool has_sub_pass = config.subpass_mask != 0;

  const VkAttachmentLoadOp kClear = VK_ATTACHMENT_LOAD_OP_CLEAR;
  const VkAttachmentLoadOp kDontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  const VkAttachmentLoadOp kKeep = VK_ATTACHMENT_LOAD_OP_LOAD;
  const VkAttachmentStoreOp kDisableStore = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  const VkAttachmentStoreOp kEnableStore = VK_ATTACHMENT_STORE_OP_STORE;

  const bool discard = (static_cast<uint32_t>(config.discard_start) &
                        static_cast<uint32_t>(TargetBufferFlags::COLOR)) != 0;
  struct { VkImageLayout subpass, initial, final; } color_layouts[MAX_SUPPORTED_RENDER_TARGET_COUNT];

  // 如果是交换链，则将第一个layout设为color attachment
  if (is_swapchain) {
    color_layouts[0].subpass = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    color_layouts[0].initial = discard ? VK_IMAGE_LAYOUT_UNDEFINED :
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    color_layouts[0].final = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  } else {
    for (int i = 0; i > MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
      color_layouts[i].subpass = config.color_layout[i];
      color_layouts[i].initial = config.color_layout[i];
      color_layouts[i].final = config.color_layout[i];
    }
  }

  VkAttachmentReference input_attachment_ref[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  // 此处代表会拥有两个SubPass，每个SubPass最多可持有MAX数量的color_attachment,
  // 对于第二个subpass，可能有部分的attachment是作为输入
  VkAttachmentReference color_attachment_refs[2][MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  VkAttachmentReference resolve_attachment_ref[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  VkAttachmentReference depth_attachment_ref = {};

  const bool has_depth = config.depth_format != VK_FORMAT_UNDEFINED;

  // todo: 为什么要用2个subpass
  VkSubpassDescription subpass[2] = {{
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .pInputAttachments = nullptr,
    .pColorAttachments = color_attachment_refs[0],
    .pResolveAttachments = resolve_attachment_ref,
    .pDepthStencilAttachment = has_depth ? &depth_attachment_ref : nullptr
  }, {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .pInputAttachments = input_attachment_ref,
    .pColorAttachments = color_attachment_refs[1],
    .pResolveAttachments = resolve_attachment_ref,
    .pDepthStencilAttachment = has_depth ? &depth_attachment_ref : nullptr
  }};

  VkAttachmentDescription attachments[MAX_SUPPORTED_RENDER_TARGET_COUNT + MAX_SUPPORTED_RENDER_TARGET_COUNT + 1] = {};

  //
  VkSubpassDependency dependencies[1] = {{
    .srcSubpass = 0,
    .dstSubpass = 1,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
    .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
  }};

  VkRenderPassCreateInfo render_pass_info {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 0u,
    .pAttachments = attachments,
    .subpassCount = has_sub_pass ? 2u : 1u,
    .pSubpasses = subpass,
    .dependencyCount = has_sub_pass ? 1u : 0u,
    .pDependencies = dependencies
  };
  // 上述的代码让我们完成了所有attachment的属性设置
  // 接下来需要对于每个subpass进行attachment的绑定


  // 该变量标识是第几个attachment（所有的attachment）
  int attachment_index = 0;
  // color attachment的绑定
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (config.color_format[i] == VK_FORMAT_UNDEFINED) {
      continue;
    }

    const VkImageLayout subpass_layout = color_layouts[i].subpass;
    uint32_t index;

    if (!has_sub_pass) {
      // 不存在subpass时，说明只使用第一个subpass
      // index为该subpass目前需要绑定的attachment
      index = subpass[0].colorAttachmentCount++;

      color_attachment_refs[0][index].layout = subpass_layout;
      color_attachment_refs[0][index].attachment = attachment_index;
    } else {
      // bitmask用来标记那些只属于第二个subpass且用于输入的attachment
      assert(config.subpass_mask == 1);

      // 如果第i位存在，则代表第i个attachment用于第二个subpass的输入
      // 即同一个attachment，是第一个subpass的输出，第二个subpass的输入
      // 将第i处的layout赋给两个subpass的layout，确保两者能对应
      if (config.subpass_mask & (1 << i)) {
        index = subpass[0].colorAttachmentCount++;
        color_attachment_refs[0][index].layout = subpass_layout;
        color_attachment_refs[0][index].attachment = attachment_index;
        // 切至第二个subpass，设置第二个subpass的color attachment
        index = subpass[1].colorAttachmentCount++;
        color_attachment_refs[1][index].layout = subpass_layout;
        color_attachment_refs[1][index].attachment = attachment_index;
      }

      // 对于非输入的情况，只设置第二个
      index = subpass[1].colorAttachmentCount++;
      color_attachment_refs[1][index].layout = subpass_layout;
      color_attachment_refs[1][index].attachment = attachment_index;
    }

    // 计算第i个color attachment对应的TargetBuffer标志位
    const TargetBufferFlags flag = TargetBufferFlags(int(TargetBufferFlags::COLOR0) << i);
    // 判断该attachment是否需要清空
    const bool clear = (static_cast<uint32_t>(config.clear) &
                        static_cast<uint32_t>(flag));
    // 判断该attachment是否需要被丢弃
    const bool discard = (static_cast<uint32_t>(config.discard_start) &
                          static_cast<uint32_t>(flag));

    //此处设置对应的attachment信息
    // loadOp为该attachment加载时需要的操作
    // samples为采样点数量,对于不止1个采样点的attachment，我们不进行保存
    // stencil：因为现在处理的是color attachment，故不处理stencil相关
    attachments[attachment_index++] = {
        .format = config.color_format[i],
        .samples = (VkSampleCountFlagBits) config.samples,
        .loadOp = clear ? kClear : (discard ? kDontCare : kKeep),
        .storeOp = config.samples == 1 ? kEnableStore : kDisableStore,
        .stencilLoadOp = kDontCare,
        .stencilStoreOp = kDisableStore,
        .initialLayout = color_layouts[i].initial,
        .finalLayout = color_layouts[i].final
    };
  }

  // 当第一个subpass没绑定任何color attachment时，将其相关属性设空
  if (subpass[0].colorAttachmentCount == 0) {
    subpass[0].pColorAttachments = nullptr;
    subpass[0].pResolveAttachments = nullptr;
    subpass[1].pColorAttachments = nullptr;
    subpass[1].pResolveAttachments = nullptr;
  }

  // 此处开始处理Resolve attachment
  // ResolveAttachment可以作为Color Attachment的降采样纹理使用，
  // 更具体的说，便是对于多重采样的Color Attachment，我们有时需要采样点数量更低的版本，
  // 此时，我们便可以使用对应的resolve attachment来方便对应的操作

  VkAttachmentReference* p_resolve_attachment = resolve_attachment_ref;
  // 以指针的方式来设置每一项resolve_attachment
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (config.color_format[i] == VK_FORMAT_UNDEFINED) {
      continue;
    }
    // 通过标识来判断哪些color attachment需要resolve attachment
    if (!(config.needs_resolve_mask & (1 << i))) {
      p_resolve_attachment->attachment = VK_ATTACHMENT_UNUSED;
      ++p_resolve_attachment;
      continue;
    }

    p_resolve_attachment->attachment = attachment_index;
    p_resolve_attachment->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ++p_resolve_attachment;
    // resolve attachment只使用一个采样点，所以这里samples只用1bit
    attachments[attachment_index++] = {
        .format = config.color_format[i],
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = kDontCare,
        .storeOp = kEnableStore,
        .stencilLoadOp = kDontCare,
        .stencilStoreOp = kDisableStore,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = color_layouts[i].final
    };
  }

  // 设置depth attachment
  if (has_depth) {
    const bool clear = (static_cast<uint32_t>(config.clear) &
                        static_cast<uint32_t>(TargetBufferFlags::DEPTH)) != 0;
    const bool discard_start = (static_cast<uint32_t>(config.discard_start) &
                                static_cast<uint32_t>(TargetBufferFlags::DEPTH)) != 0;
    const bool discard_end = (static_cast<uint32_t>(config.discard_end) &
                              static_cast<uint32_t>(TargetBufferFlags::DEPTH)) != 0;
    depth_attachment_ref.layout = config.depth_layout;
    depth_attachment_ref.attachment = attachment_index;
    attachments[attachment_index++] = {
        .format = config.depth_format,
        .samples = (VkSampleCountFlagBits) config.samples,
        .loadOp = clear ? kClear : (discard_start ? kDontCare : kKeep),
        .storeOp = discard_end ? kDisableStore : kEnableStore,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = kDisableStore,
        .initialLayout = config.depth_layout,
        .finalLayout = config.depth_layout
    };
  }

  // 设置renderpass
  render_pass_info.attachmentCount = attachment_index;

  VkDevice* device = VulkanContext::Get().device_;
  VkRenderPass render_pass;
  VkResult result = vkCreateRenderPass(
      *device, &render_pass_info, nullptr, &render_pass);
  if (result != VK_SUCCESS) {
    LOG_ERROR("VulkanFBOCache", "Create RenderPass Failed! code:{}", result);
  }

  LOG_INFO("VulkanFBOCache", "Create RenderPass :{},\n"
                             "samples = {},\n"
                             "depth = {},\n"
                             "attachment_cnt = {}",
                             (void*)render_pass,
                             config.samples,
                             has_depth ? 1 : 0,
                             attachment_index);
  return render_pass;
}

void VulkanFBOCache::Reset() {
  // 清空framebuffer
  VkDevice* device = VulkanContext::Get().device_;
  for (auto& pair : frame_buffer_cache_) {
    renderpass_ref_cnt_[pair.first.render_pass]--;
    vkDestroyFramebuffer(*device, pair.second.handle, nullptr);
  }
  frame_buffer_cache_.clear();

  for (auto& pair : render_pass_cache_) {
    vkDestroyRenderPass(*device, pair.second.handle, nullptr);
  }
  render_pass_cache_.clear();
}

void VulkanFBOCache::GC() {
  // 帧太少时不需要清理
  if (++current_time <= TIME_BEFORE_EVICTION) {
    return;
  }

  const uint32_t evict_time = current_time - TIME_BEFORE_EVICTION;

}

}