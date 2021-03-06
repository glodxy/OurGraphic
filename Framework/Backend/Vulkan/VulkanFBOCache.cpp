//
// Created by Glodxy on 2021/9/24.
//
#include <functional>
#include "VulkanFBOCache.h"
#include "VulkanContext.h"

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

std::size_t VulkanFBOCache::FBOKeyHash::operator()(const FBOKey &key) const {
  std::string addr = std::to_string((size_t)&key);
  std::hash<std::string> hasher;
  return hasher(addr);
}

std::size_t VulkanFBOCache::RenderPassHash::operator()(const RenderPassKey &key) const {
  std::string addr = std::to_string((size_t)&key);
  std::hash<std::string> hasher;
  return hasher(addr);
}

VulkanFBOCache::VulkanFBOCache() {}

VulkanFBOCache::~VulkanFBOCache() {

}

VkFramebuffer VulkanFBOCache::GetFrameBuffer(FBOKey config) noexcept {
  auto framebuffer_iter = frame_buffer_cache_.find(config);
  // ?????????????????????????????????????????????????????????
  if ((framebuffer_iter != frame_buffer_cache_.end()) &&
      (framebuffer_iter->second.handle != VK_NULL_HANDLE)) {
    framebuffer_iter->second.timestamp = current_time;
    return framebuffer_iter->second.handle;
  }

  VkImageView attachments[MAX_SUPPORTED_RENDER_TARGET_COUNT + MAX_SUPPORTED_RENDER_TARGET_COUNT + 1];
  uint32_t attachment_count = 0;
  // ?????????????????????
  for (auto attachment : config.color) {
    if (attachment) {
      attachments[attachment_count++] = attachment;
    }
  }
  // ???????????????????????????
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

  // ??????????????????????????????buffer
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

  // ????????????????????????????????????layout??????color attachment
  if (is_swapchain) {
    color_layouts[0].subpass = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    color_layouts[0].initial = discard ? VK_IMAGE_LAYOUT_UNDEFINED :
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    color_layouts[0].final = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  } else {
    for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
      color_layouts[i].subpass = config.color_layout[i];
      color_layouts[i].initial = config.color_layout[i];
      color_layouts[i].final = config.color_layout[i];
    }
  }

  VkAttachmentReference input_attachment_ref[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  // ???????????????????????????SubPass?????????SubPass???????????????MAX?????????color_attachment,
  // ???????????????subpass?????????????????????attachment???????????????
  VkAttachmentReference color_attachment_refs[2][MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  VkAttachmentReference resolve_attachment_ref[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  VkAttachmentReference depth_attachment_ref = {};

  const bool has_depth = config.depth_format != VK_FORMAT_UNDEFINED;


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
  // ???????????????????????????????????????attachment???????????????
  // ???????????????????????????subpass??????attachment?????????


  // ???????????????????????????attachment????????????attachment???
  int attachment_index = 0;
  // color attachment?????????
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (config.color_format[i] == VK_FORMAT_UNDEFINED) {
      continue;
    }

    const VkImageLayout subpass_layout = color_layouts[i].subpass;
    uint32_t index;

    if (!has_sub_pass) {
      // ?????????subpass??????????????????????????????subpass
      // index??????subpass?????????????????????attachment
      index = subpass[0].colorAttachmentCount++;

      color_attachment_refs[0][index].layout = subpass_layout;
      color_attachment_refs[0][index].attachment = attachment_index;
    } else {
      // bitmask????????????????????????????????????subpass??????????????????attachment
      assert(config.subpass_mask == 1);

      // ?????????i????????????????????????i???attachment???????????????subpass?????????
      // ????????????attachment???????????????subpass?????????????????????subpass?????????
      // ??????i??????layout????????????subpass???layout????????????????????????
      if (config.subpass_mask & (1 << i)) {
        index = subpass[0].colorAttachmentCount++;
        color_attachment_refs[0][index].layout = subpass_layout;
        color_attachment_refs[0][index].attachment = attachment_index;
        // ???????????????subpass??????????????????subpass???color attachment
        index = subpass[1].colorAttachmentCount++;
        color_attachment_refs[1][index].layout = subpass_layout;
        color_attachment_refs[1][index].attachment = attachment_index;
      }

      // ?????????????????????????????????????????????
      index = subpass[1].colorAttachmentCount++;
      color_attachment_refs[1][index].layout = subpass_layout;
      color_attachment_refs[1][index].attachment = attachment_index;
    }

    // ?????????i???color attachment?????????TargetBuffer?????????
    const TargetBufferFlags flag = TargetBufferFlags(int(TargetBufferFlags::COLOR0) << i);
    // ?????????attachment??????????????????
    const bool clear = (static_cast<uint32_t>(config.clear) &
                        static_cast<uint32_t>(flag));
    // ?????????attachment?????????????????????
    const bool discard = (static_cast<uint32_t>(config.discard_start) &
                          static_cast<uint32_t>(flag));

    //?????????????????????attachment??????
    // loadOp??????attachment????????????????????????
    // samples??????????????????,????????????1???????????????attachment????????????????????????
    // stencil???????????????????????????color attachment???????????????stencil??????
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

  // ????????????subpass???????????????color attachment??????????????????????????????
  if (subpass[0].colorAttachmentCount == 0) {
    subpass[0].pColorAttachments = nullptr;
    subpass[0].pResolveAttachments = nullptr;
    subpass[1].pColorAttachments = nullptr;
    subpass[1].pResolveAttachments = nullptr;
  }

  // ??????????????????Resolve attachment
  // ResolveAttachment????????????Color Attachment???????????????????????????
  // ?????????????????????????????????????????????Color Attachment??????????????????????????????????????????????????????
  // ???????????????????????????????????????resolve attachment????????????????????????

  VkAttachmentReference* p_resolve_attachment = resolve_attachment_ref;
  // ????????????????????????????????????resolve_attachment
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (config.color_format[i] == VK_FORMAT_UNDEFINED) {
      continue;
    }
    // ???????????????????????????color attachment??????resolve attachment
    if (!(config.needs_resolve_mask & (1 << i))) {
      p_resolve_attachment->attachment = VK_ATTACHMENT_UNUSED;
      ++p_resolve_attachment;
      continue;
    }

    p_resolve_attachment->attachment = attachment_index;
    p_resolve_attachment->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ++p_resolve_attachment;
    // resolve attachment???????????????????????????????????????samples??????1bit
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

  // ??????depth attachment
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

  // ??????renderpass
  render_pass_info.attachmentCount = attachment_index;

  VkDevice* device = VulkanContext::Get().device_;
  VkRenderPass render_pass;
  VkResult result = vkCreateRenderPass(
      *device, &render_pass_info, nullptr, &render_pass);
  if (result != VK_SUCCESS) {
    LOG_ERROR("VulkanFBOCache", "Create RenderPass Failed! code:{}", result);
  }
  render_pass_cache_[config] = {render_pass, current_time};
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
  // ??????framebuffer
  VkDevice* device = VulkanContext::Get().device_;
  for (auto& pair : frame_buffer_cache_) {
    renderpass_ref_cnt_[pair.first.render_pass]--;
    vkDestroyFramebuffer(*device, pair.second.handle, nullptr);
  }
  frame_buffer_cache_.clear();

  for (auto pair : render_pass_cache_) {
    vkDestroyRenderPass(*device, pair.second.handle, nullptr);
  }
  render_pass_cache_.clear();
}

void VulkanFBOCache::GC() {
  // ???????????????????????????
  if (++current_time <= TIME_BEFORE_EVICTION) {
    return;
  }

  const uint32_t evict_time = current_time - TIME_BEFORE_EVICTION;

}

}