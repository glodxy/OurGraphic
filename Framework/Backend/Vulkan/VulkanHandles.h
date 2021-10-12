//
// Created by Glodxy on 2021/10/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANHANDLES_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANHANDLES_H_

#include "../include_internal/IResource.h"
#include "../include_internal/DriverEnum.h"
#include "Framework/Backend/include/Program.h"
#include "Framework/Backend/include/SamplerGroup.h"
#include "VulkanContext.h"
#include "VulkanTexture.h"
#include "VulkanPipelineCache.h"

namespace our_graph {
struct VulkanBuffer;

class VulkanShader : public IShader {
 public:
  VulkanShader(const Program& program) noexcept;
  VulkanShader(VkShaderModule vs, VkShaderModule fs) noexcept;
  ~VulkanShader();

  VulkanPipelineCache::ProgramBundle bundle_;
  Program::SamplerGroupInfo sampler_group_info_;
};

class VulkanSamplerGroup : public ISamplerGroup {
 public:
  VulkanSamplerGroup(uint32_t count) :
        ISamplerGroup(count), sampler_group_(new SamplerGroup(count)) {}

  std::unique_ptr<SamplerGroup> sampler_group_;
};

class VulkanRenderTarget : private IRenderTarget {
 public:
  VulkanRenderTarget(uint32_t width, uint32_t height, uint8_t samples,
                     VulkanAttachment color[MAX_SUPPORTED_RENDER_TARGET_COUNT],
                     VulkanAttachment depth_stencil[2],
                     VulkanStagePool& stage_pool);

  explicit VulkanRenderTarget();

  void TransformClientRectToPlatform(VulkanSwapChain* current_surface, VkRect2D* bounds) const;
  void TransformClientRectToPlatform(VulkanSwapChain* current_surface, VkViewport* bounds) const;

  VkExtent2D GetExtent(VulkanSwapChain* current_surface) const;
  VulkanAttachment GetColor(VulkanSwapChain* current_surface, int target) const;
  VulkanAttachment GetMsaaColor(int target) const;
  VulkanAttachment GetDepth(VulkanSwapChain* current_surface) const;
  VulkanAttachment GetMsaaDepth() const;

  int GetColorTargetCount(const VulkanRenderPass& pass) const;
  uint8_t GetSamples() const {return samples_;}
  bool HasDepth() const {return depth_.format != VK_FORMAT_UNDEFINED;}
  bool IsSwapChain() const {return !offscreen_;}

 private:
  VulkanAttachment color_[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  VulkanAttachment depth_ = {};
  VulkanAttachment msaa_attachments_[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
  VulkanAttachment msaa_depth_attachment_ = {};
  const bool offscreen_ : 1; // 是否有用于显示： 1:未使用
  uint8_t samples_ : 7;  // 采样数
};

class VulkanVertexBuffer : public IVertexBuffer {
 public:
  VulkanVertexBuffer(VulkanStagePool& stage_pool, uint8_t buffer_cnt,
                     uint8_t attr_cnt, uint32_t element_cnt,
                     const AttributeArray& attributes);
  std::vector<const VulkanBuffer* > buffers_;
};

class VulkanIndexBuffer :public IIndexBuffer {
 public:
  VulkanIndexBuffer(VulkanStagePool& stage_pool, uint8_t element_size,
                    uint32_t index_cnt);
  ~VulkanIndexBuffer();

  VulkanBuffer* buffer_;
  const VkIndexType index_type_;
};

class VulkanRenderPrimitive : public IRenderPrimitive {
 public:
  // 设置光栅化类型
  void SetPrimitiveType(PrimitiveType type);

  void SetBuffers(VulkanVertexBuffer* vertex, VulkanIndexBuffer* index);

  VulkanVertexBuffer* vertex_buffer_ {nullptr};
  VulkanIndexBuffer* index_buffer_ {nullptr};
  VkPrimitiveTopology primitive_topology_;
};

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANHANDLES_H_
