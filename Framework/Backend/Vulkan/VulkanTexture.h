//
// Created by Glodxy on 2021/10/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTURE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTURE_H_
#include "VulkanDef.h"
#include "../include/PixelBufferDescriptor.h"
#include "../include_internal/IResource.h"
#include "VulkanStagePool.h"
namespace our_graph {

/**
 * 该类维护了一份CPU中的buffer，
 * 只有在需要时生成相应的image view，
 * 所以一个图片可以对应多个view
 * */

class VulkanTexture : public ITexture {
 public:
  VulkanTexture(SamplerType sampler_type, uint8_t levels,
                TextureFormat format, uint8_t samples,
                uint32_t width, uint32_t height, uint32_t depth,
                TextureUsage usage, VulkanStagePool& stage_pool,
                VkComponentMapping swizzle = {});

  ~VulkanTexture();

  void Update2DImage(const PixelBufferDescriptor& data, uint32_t width,
                     uint32_t height, int mip_levels);

  void Update3DImage(const PixelBufferDescriptor& data, uint32_t width,
                     uint32_t height, uint32_t depth, int mip_levels);
  void UpdateCubeImage(const PixelBufferDescriptor& data, const FaceOffsets& face_offsets,
                       int mip_levels);

  /**
   * 获取primary的imageview,
   * 该view只能用来生成sampler
   * */
  VkImageView GetPrimaryImageView() const {
    return cached_image_views_.at(primary_view_range_);
  }

  /**
   * 设置image view的mipmap的层次范围
   * */
  void SetPrimaryRange(uint32_t min_mip_level, uint32_t max_mip_level);


  /**
   * 获取imageview
   * 该view可用于render target
   * 与Primary的差别在于该view始终是2D
   * */
  VkImageView GetAttachmentView(int single_level, int single_layer, VkImageAspectFlags aspect);

  VkFormat GetVKFormat() const {
    return vk_format_;
  }

  VkImage GetVKImage() const {
    return texture_image_;
  }

  void SetMSAA(VulkanTexture * msaa) {
    msaa_ = msaa;
  }
  VulkanTexture* GetMSAA() const {
    return msaa_;
  }

 private:
  /**
   * 获取或创建一个带有mipmap的ImageView
   * */
  VkImageView GetImageView(VkImageSubresourceRange range, bool is_attachment = false);

  void CopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image,
                         uint32_t width, uint32_t height, uint32_t depth,
                         const FaceOffsets* face_offsets, int mip_level);

  void UpdateWithBlitImage(const PixelBufferDescriptor& host_data, uint32_t width,
                           uint32_t height, uint32_t depth, int mip_level);

  void UpdateWithCopyBuffer(const PixelBufferDescriptor& host_data, uint32_t width,
                            uint32_t height, uint32_t depth, int mip_level);

  VulkanTexture* msaa_ = nullptr; //多重采样的纹理
  const VkFormat vk_format_;
  const VkComponentMapping swizzle_;
  VkImageViewType view_type_;
  VkImage texture_image_ = VK_NULL_HANDLE;
  VkDeviceMemory texture_image_memory_ = VK_NULL_HANDLE;
  VkImageSubresourceRange primary_view_range_;
  VkImageAspectFlags  aspect_;
  VulkanStagePool& stage_pool_;

  /**
   * 该map用于缓存已建立的image view
   * */
  std::map<VkImageSubresourceRange, VkImageView> cached_image_views_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTURE_H_
