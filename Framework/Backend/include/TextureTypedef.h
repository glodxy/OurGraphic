//
// Created by Glodxy on 2021/9/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_TEXTURETYPEDEF_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_TEXTURETYPEDEF_H_
// 临时引入vulkan头文件
// todo：将Vulkan类型换为自定类型
#include "vulkan/vulkan.h"
namespace our_graph {
namespace texture {
typedef uint64_t CreateStorageFlags;
typedef uint64_t SampleCountFlags;
typedef uint64_t UsageFlags;
typedef uint32_t MipLevel;
typedef uint32_t ArrayLayers;
typedef VkFormat Format;
typedef VkImageLayout ImageLayout;

enum StorageFlag {
  // vulkan
  SPARSE_BINDING = 0x00000001, // 稀疏内存绑定
  SPARSE_RESIDENCY = 0x00000002, // 部分使用稀疏内存绑定
  SPARSE_ALIASED = 0x00000004, // 图像完全使用稀疏内存，并分块保存
  MUTABLE_FORMAT = 0x00000008, // 图像与视图的格式不统一
  CUBE_COMPATIBLE = 0x00000010, // 专用于立方体映射的存储
};

enum SampleCountFlag {
  // vulkan
  VK_SAMPLE_COUNT_1_BIT = 0x00000001,
  VK_SAMPLE_COUNT_2_BIT = 0x00000002,
  VK_SAMPLE_COUNT_4_BIT = 0x00000004,
  VK_SAMPLE_COUNT_8_BIT = 0x00000008,
  VK_SAMPLE_COUNT_16_BIT = 0x00000010,
  VK_SAMPLE_COUNT_32_BIT = 0x00000020,
  VK_SAMPLE_COUNT_64_BIT = 0x00000040,
  VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF,
};

// 内存中重复贴图的方式
enum ImageTilingType {
  IMAGE_TILING_OPTIMAL = 0,
  IMAGE_TILING_LINEAR = 1,
  IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT = 1000158000,
  IMAGE_TILING_MAX_ENUM = 0x7FFFFFFF
};

// 图像用途
enum UsageFlag {
  // vulkan
  USAGE_TRANSFER_SRC = 0x00000001,
  USAGE_TRANSFER_DST = 0x00000002,
  USAGE_SAMPLED = 0x00000004,
  USAGE_STORAGE = 0x00000008,
  USAGE_COLOR_ATTACHMENT = 0x00000010,
  USAGE_DEPTH_STENCIL = 0x00000020,
  USAGE_TRANSIENT_ATTACHMENT = 0x00000040,
  USAGE_INPUT_ATTACHMENT = 0x00000080,
};

// vulkan:在多个队列族之间的共享模式
enum ShareMode {
  SHARING_MODE_EXCLUSIVE = 0,
  SHARING_MODE_CONCURRENT = 1,
};

// 图像的维度
enum Dimension {
  DIM_1D,
  DIM_2D,
  DIM_3D,
};

// mipmap基础级别的每个维度的元素数量
struct MipmapExtend3D {
  uint32_t width;
  uint32_t height;
  uint32_t depth;
};

struct TextureCreateInfo {
  Dimension type_; // 图像类型
  Format format_; // 图像格式
  CreateStorageFlags flags_ {SPARSE_BINDING}; // 按位保存的图像存储信息
  MipmapExtend3D extend_; // mipmap基础级别的每个维度的元素数量
  MipLevel mip_levels_; // mipmap采样级数
  ArrayLayers array_layers_; // 图像数组的层数
  SampleCountFlags sample_flags_; // 子元素采样数
  ImageTilingType tiling_type_; // 重复贴图类型
  UsageFlags usage_flags_; // 图像的用途标志
  ShareMode share_mode_ {SHARING_MODE_EXCLUSIVE};
  uint32_t queue_family_index_cnt_;
  ImageLayout layout_{VK_IMAGE_LAYOUT_PREINITIALIZED};
};

}
}
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_TEXTURETYPEDEF_H_
