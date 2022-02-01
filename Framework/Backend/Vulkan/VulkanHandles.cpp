//
// Created by Glodxy on 2021/10/9.
//
#include "VulkanDef.h"
#include "VulkanHandles.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include <utility>


namespace our_graph {

static void FlipVertically(VkRect2D* rect, uint32_t framebuffer_height) {
  // 底点减去总高度
  rect->offset.y = framebuffer_height - rect->offset.y - rect->extent.height;
}

static void FlipVertically(VkViewport* rect, uint32_t framebuffer_height) {
  rect->y = framebuffer_height - rect->y;
  rect->height = -rect->height;
  //rect->y = framebuffer_height - rect->y - rect->height;
}

/**
 * 将rect放缩至framebuffer范围内
 * */
static void ClampToFrameBuffer(VkRect2D* rect, uint32_t fb_width, uint32_t fb_height) {
  int32_t x = std::max(rect->offset.x, 0);
  int32_t y = std::max(rect->offset.y, 0);
  int32_t right = std::min(rect->offset.x + (int32_t)rect->extent.width, (int32_t) fb_width);
  int32_t top = std::min(rect->offset.y + (int32_t)rect->extent.height, (int32_t)fb_height);
  rect->offset.x = std::min(x, (int32_t)fb_width);
  rect->offset.y = std::min(y, (int32_t)fb_height);
  rect->extent.width = std::max(right - x, 0);
  rect->extent.height = std::max(top - y, 0);
}

/////////////////////Shader////////////////////
VulkanShader::VulkanShader(const Program &program) noexcept : IShader(program.GetName()) {
  const auto& blobs = program.GetShadersSource();
  VkShaderModule * modules[2] = { &bundle_.vertex, &bundle_.fragment};
  bool missing = false;
  for (int i = 0; i < Program::SHADER_TYPE_COUNT; ++i) {\
    const auto& blob = blobs[i];
    VkShaderModule *module = modules[i];
    if (blob.empty()) {
      missing = true;
      continue;
    }
    VkShaderModuleCreateInfo module_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = blob.size(),
        .pCode = (uint32_t*) blob.data(),
    };
    VkResult res = vkCreateShaderModule(*VulkanContext::Get().device_,
                                        &module_info,
                                        nullptr,
                                        module);
    CHECK_RESULT(res, "VulkanShader", "Failed To Create Shader Module!");
    assert(res == VK_SUCCESS);
  }

  if (missing) {
    LOG_WARN("VulkanShader", "Missing Shader:{}", program.GetName());
  }

  sampler_group_info_ = program.GetSamplerGroupInfo();

  LOG_INFO("VulkanShader", "Created VulkanShader:{}!", program.GetName());
}

VulkanShader::VulkanShader(VkShaderModule vs, VkShaderModule fs) noexcept {
  bundle_.vertex = vs;
  bundle_.fragment = fs;
}

VulkanShader::~VulkanShader() {
  vkDestroyShaderModule(*VulkanContext::Get().device_, bundle_.vertex, nullptr);
  vkDestroyShaderModule(*VulkanContext::Get().device_, bundle_.fragment, nullptr);
  bundle_.vertex = VK_NULL_HANDLE;
  bundle_.fragment = VK_NULL_HANDLE;
}
/**
 * 根据已有attachment拷贝attachment
 * */
static VulkanAttachment CreateAttachment(VulkanAttachment spec) {
  if (spec.texture == nullptr) {
    return spec;
  }
  return {
    .format = spec.texture->GetVKFormat(),
    .image = spec.texture->GetVKImage(),
    .view = {},
    .memory = {},
    .texture = spec.texture,
    .layout = VulkanUtils::GetTextureLayout(spec.texture->usage_),
    .level = spec.level,
    .layer = spec.layer
  };
}

VulkanRenderTarget::VulkanRenderTarget() : IRenderTarget(0, 0),
    offscreen_(false), samples_(1) {

}

/**
 * @param depth_stencil: [0]:depth, [1]:stencil
 * */
VulkanRenderTarget::VulkanRenderTarget(uint32_t width,
                                       uint32_t height,
                                       uint8_t samples,
                                       VulkanAttachment *color,
                                       VulkanAttachment *depth_stencil,
                                       VulkanStagePool &stage_pool) :
    IRenderTarget(width, height), offscreen_(true), samples_(samples) {

  /**
   * 对每一个attachment创建相应的view
   * */
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    const VulkanAttachment& spec = color[i];
    color_[i] = CreateAttachment(spec);
    VulkanTexture* texture = spec.texture;
    if (!texture) {
      continue;
    }
    color_[i].view = texture->GetAttachmentView(spec.level, spec.layer,
                                                VK_IMAGE_ASPECT_COLOR_BIT);
  }

  const VulkanAttachment& depth_spec = depth_stencil[0];
  depth_ = CreateAttachment(depth_spec);
  VulkanTexture* depth_texture = depth_.texture;
  if (depth_texture) {
    depth_.view = depth_texture->GetAttachmentView(depth_.level, depth_.layer,
                                                   VK_IMAGE_ASPECT_DEPTH_BIT);
  }

  if (samples == 1) {
    // 采样数为1，不管
    return;
  }

  const auto& limits = VulkanContext::Get().physical_device_properties_.limits;
  samples_ = samples =
      VulkanUtils::ReduceSampleCount(
          samples,
          limits.framebufferDepthSampleCounts & limits.framebufferColorSampleCounts);

  const int level = 1;
  const int depth = 1;

  // 创建msaa纹理
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    const VulkanAttachment& spec = color[i];
    VulkanTexture* texture = spec.texture;
    // 采样数为1，需要手动创建
    if (texture && texture->samples_ == 1) {
      VulkanTexture* ms_texture = texture->GetMSAA();
      if (!ms_texture) {
        ms_texture = new VulkanTexture(texture->sampler_type_, level,
                                       texture->format_, samples, width, height,
                                       depth, texture->usage_, stage_pool);
        texture->SetMSAA(ms_texture);
      }
      msaa_attachments_[i] = CreateAttachment({.texture = ms_texture});
      msaa_attachments_[i].view = ms_texture->GetAttachmentView(0, 0,
                                                                VK_IMAGE_ASPECT_COLOR_BIT);
    }
    if (texture && texture->samples_ > 1) {
      msaa_attachments_[i] = color_[i];
    }
  }

  if (!depth_texture) {
    return;
  }

  if (depth_texture->samples_ > 1) {
    msaa_depth_attachment_ = depth_;
    return;
  }

  // 否则创建深度纹理的msaa
  VulkanTexture* depth_ms_texture = depth_texture->GetMSAA();
  if (!depth_ms_texture) {
    depth_ms_texture = new VulkanTexture(depth_texture->sampler_type_, level,
                                         depth_texture->format_, samples, width, height,
                                         depth, depth_texture->usage_, stage_pool);
    depth_texture->SetMSAA(depth_ms_texture);
  }

  msaa_depth_attachment_ = CreateAttachment(
      {.texture = depth_ms_texture,
       .layout = {},
       .level = depth_spec.level,
       .layer = depth_spec.layer});
  msaa_depth_attachment_.view = depth_ms_texture->GetAttachmentView(
      depth_spec.level, depth_spec.layer, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRenderTarget::TransformClientRectToPlatform(VulkanSwapChain *current_surface, VkRect2D *bounds) const {
  const auto& extent = GetExtent(current_surface);
  FlipVertically(bounds, extent.height);
  ClampToFrameBuffer(bounds, extent.width, extent.height);
}

void VulkanRenderTarget::TransformClientRectToPlatform(VulkanSwapChain *current_surface, VkViewport *bounds) const {
  const auto& extent = GetExtent(current_surface);
  FlipVertically(bounds, extent.height);
}

VkExtent2D VulkanRenderTarget::GetExtent(VulkanSwapChain *current_surface) const {
  if (offscreen_) {
    return {width_, height_};
  }

  return current_surface->GetClientSize();
}

VulkanAttachment VulkanRenderTarget::GetColor(VulkanSwapChain *current_surface, int target) const {
  return (offscreen_ || target > 0) ? color_[target] : current_surface->GetColor();
}

VulkanAttachment VulkanRenderTarget::GetDepth(VulkanSwapChain *current_surface) const {
  return offscreen_ ? depth_ : current_surface->GetDepth();
}

VulkanAttachment VulkanRenderTarget::GetMsaaColor(int target) const {
  return msaa_attachments_[target];
}

VulkanAttachment VulkanRenderTarget::GetMsaaDepth() const {
  return msaa_depth_attachment_;
}

int VulkanRenderTarget::GetColorTargetCount(const VulkanRenderPass &pass) const {
  if (!offscreen_) {
    return 1;
  }
  int cnt = 0;
  for (int i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (color_[i].format == VK_FORMAT_UNDEFINED) {
      continue;
    }
    // 判断当前pass有没有在使用
    if (!(pass.subpassMask & (1 << i)) || pass.currentSubpass == 1) {
      cnt++;
    }
  }
  return cnt;
}
////////////////////////////////Buffer//////////////
VulkanVertexBuffer::VulkanVertexBuffer(VulkanStagePool &stage_pool,
                                       uint8_t buffer_cnt,
                                       uint8_t attr_cnt,
                                       uint32_t element_cnt,
                                       const AttributeArray &attributes) :
                                       IVertexBuffer(buffer_cnt, attr_cnt, element_cnt, attributes),
                                       buffers_(buffer_cnt, nullptr){

}

VulkanIndexBuffer::VulkanIndexBuffer(VulkanStagePool &stage_pool,
                                     uint8_t element_size,
                                     uint32_t index_cnt) :
                                     IIndexBuffer(element_size, index_cnt),
                                     index_type_(element_size == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32) {
  buffer_ = new VulkanBuffer(stage_pool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                            element_size * index_cnt);

}

VulkanIndexBuffer::~VulkanIndexBuffer() {
  buffer_->Destroy();
  delete buffer_;
}

VulkanBufferObject::VulkanBufferObject(VulkanStagePool &stage_pool,
                                       uint32_t byte_count,
                                       BufferObjectBinding bind_type,
                                       BufferUsage usage) :
    IBufferObject(byte_count),
    bind_type_(bind_type) {
  VkBufferUsageFlagBits vk_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (bind_type_ == BufferObjectBinding::UNIFORM) {
    vk_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }
  buffer_ = new VulkanBuffer(
      stage_pool, vk_usage, byte_count);
}

VulkanBufferObject::~VulkanBufferObject() {
  buffer_->Destroy();
  delete buffer_;
}
///////////////////////////////////////////////
////////////////Primitive//////////////////
void VulkanRenderPrimitive::SetPrimitiveType(PrimitiveType type) {
  type_ = type;
  switch (type) {
    case PrimitiveType::NONE:
    case PrimitiveType::POINTS:
      primitive_topology_ = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      break;
    case PrimitiveType::LINES:
      primitive_topology_ = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      break;
    case PrimitiveType::TRIANGLES:
      primitive_topology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      break;
  }
}

void VulkanRenderPrimitive::SetBuffers(VulkanVertexBuffer *vertex, VulkanIndexBuffer *index) {
  vertex_buffer_ = vertex;
  index_buffer_ = index;
}
}  // namespace our_graph