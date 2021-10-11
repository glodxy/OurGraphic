//
// Created by Glodxy on 2021/9/24.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANFBOCACHE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANFBOCACHE_H_
#include <unordered_map>

#include "../include_internal/DriverEnum.h"
#include "VulkanDef.h"
namespace our_graph {
/**
 * 该类为FrameBufferObject的缓存
 * 管理了每帧的数据以及相关的RenderPass
 * */
class VulkanFBOCache {
 public:
  explicit VulkanFBOCache();
  ~VulkanFBOCache();

 public:
  // 按照8字节对齐
  // 该结构体描述了RenderPass的属性
  struct alignas(8) RenderPassKey {
    // 颜色缓存， 有8个，每个4字节
    VkImageLayout color_layout[MAX_SUPPORTED_RENDER_TARGET_COUNT];
    // 每个颜色缓存对应的格式
    VkFormat color_format[MAX_SUPPORTED_RENDER_TARGET_COUNT];
    // 深度缓存的布局 4字节
    VkImageLayout depth_layout;
    // 深度缓存的格式 4字节
    VkFormat depth_format;

    // 清除的颜色缓存 4字节
    TargetBufferFlags clear;
    TargetBufferFlags discard_start;
    TargetBufferFlags discard_end;

    uint8_t samples;
    uint8_t needs_resolve_mask;
    uint8_t subpass_mask;
    uint8_t padding;
  };
  struct RenderPassVal {
    VkRenderPass handle;
    uint32_t timestamp;
  };
  struct RenderPassHash {
    std::size_t operator()(const RenderPassKey& key) const;
  };
  struct RenderPassEq {
    bool operator()(const RenderPassKey& k1, const RenderPassKey& k2) const;
  };


  // 8字节对齐
  // 该结构体描述了一个FrameBuffer的属性
  struct alignas(8) FBOKey {
    // 8 字节
    VkRenderPass render_pass;

    // 2字节
    uint16_t width;
    uint16_t height;
    uint16_t layers;
    uint16_t samples;

    // 8*8字节
    VkImageView color[MAX_SUPPORTED_RENDER_TARGET_COUNT];
    // 8*8字节
    VkImageView resolve[MAX_SUPPORTED_RENDER_TARGET_COUNT];
    VkImageView depth;
  };
  struct FBOVal {
    VkFramebuffer handle;
    uint32_t timestamp;
  };
  struct FBOKeyHash {
    std::size_t operator()(const FBOKey& key) const;
  };
  struct FBOKeyEq {
    bool operator()(const FBOKey& k1, const FBOKey& k2) const;
  };

  /**
   * 根据config获取帧缓冲
   * */
  VkFramebuffer GetFrameBuffer(FBOKey config) noexcept;
  VkRenderPass GetRenderPass(RenderPassKey config) noexcept;

  void Reset();

  void GC();
 protected:

 private:
  uint32_t current_time = 0;

  std::unordered_map<FBOKey, FBOVal, FBOKeyHash, FBOKeyEq> frame_buffer_cache_;
  std::unordered_map<RenderPassKey, RenderPassVal, RenderPassHash, RenderPassEq> render_pass_cache_;
  // 该map记录了每个renderpass的引用次数
  std::unordered_map<VkRenderPass, uint32_t> renderpass_ref_cnt_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANFBOCACHE_H_
