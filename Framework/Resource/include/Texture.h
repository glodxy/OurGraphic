//
// Created by Glodxy on 2021/10/27.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_TEXTURE_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_TEXTURE_H_
#include "Backend/include/Driver.h"
#include "Backend/include/DriverEnum.h"
#include "Backend/include/PixelBufferDescriptor.h"
#include "Framework/Resource/base/BuilderBase.h"
#include "Framework/Resource/include_internal/ResourceBase.h"
namespace our_graph {

class Texture : public ResourceBase {
  struct Detail;

 public:
  static constexpr const size_t BASE_LEVEL = 0;

  using PixelBufferDescriptor = our_graph::PixelBufferDescriptor;
  using Sampler = our_graph::SamplerType;
  using InternalFormat = our_graph::TextureFormat;
  using CubemapFace = our_graph::TextureCubemapFace;
  using Format = our_graph::PixelDataFormat;
  using Type = our_graph::PixelDataType;
  using CompressedType = our_graph::CompressedPixelDataType;
  using FaceOffsets = our_graph::FaceOffsets;
  using Usage = our_graph::TextureUsage;
  using Swizzle = our_graph::TextureSwizzle;

  static bool IsTextureFormatSupported(Driver* driver, InternalFormat format) noexcept;

  static bool IsTextureSwizzleSupported(Driver* driver) noexcept;
  // 计算CPU部分需要的缓冲区大小
  static size_t ComputeTextureDataSize(Format format, Type type,
                                        size_t stride, size_t height, size_t alignment) noexcept;
  // 计算一个像素所需要的字节数
  static size_t GetFormatSize(InternalFormat format);
  /**
   * 获取指定层级的深度或宽度值
   * @param base_level_value：最低层级的对应值
   * @param level：层级数
   * @note:如获取level为3的mipmap的宽度，已知纹理宽度300，
   * 则调用ValueOfLevel(3, 300)
   * */
  static inline size_t ValueOfLevel(uint8_t level, size_t base_level_value);
  /**
   * 根据数据获取相应的层级
   * @param max_dimension：最大维度的数据，如宽度等
   * @note：如纹理800*600，则使用GetMaxLevel(800)来获取层级
   * */
  static inline uint8_t GetMaxLevel(uint32_t max_dimension);
  /**
   * 根据宽高获取对应的层级
   * 本质与上一个接口没有区别，忽略了手动取最大的过程
   * */
  static inline uint8_t GetMaxLevel(uint32_t width, uint32_t height);

  class Builder : public BuilderBase<Detail> {
    friend class Detail;
    friend class Texture;

   public:
    Builder(Driver* driver) noexcept;
    Builder(const Builder& rhs) noexcept;
    Builder(Builder&& rhs) noexcept;
    ~Builder() noexcept;
    Builder& operator=(const Builder& rhs) noexcept;
    Builder& operator=(Builder&& rhs) noexcept;

    Builder& Width(uint32_t width) noexcept;

    Builder& Height(uint32_t height) noexcept;

    Builder& Depth(uint32_t depth) noexcept;

    Builder& Levels(uint8_t levels) noexcept;

    Builder& Sampler(Sampler target) noexcept;

    Builder& Format(InternalFormat format) noexcept;

    Builder& Usage(Usage usage) noexcept;

    Builder& Swizzle(Swizzle r, Swizzle g,
                     Swizzle b, Swizzle a) noexcept;

    Texture* Build();

  };
  // 获取目标层级的宽度
  size_t GetWidth(size_t level = BASE_LEVEL) const noexcept;
  // 获取目标层级的高度
  size_t GetHeight(size_t level = BASE_LEVEL) const noexcept;
  // 获取目标层级的深度
  size_t GetDepth(size_t level = BASE_LEVEL) const noexcept;
  // 获取层级数
  size_t GetLevels() const noexcept;
  // 获取最大层级数
  size_t GetMaxLevels() const noexcept;
  // 获取采样类型
  Sampler GetSamplerType() const noexcept;
  // 获取纹理格式
  InternalFormat GetFormat() const noexcept;
  // 获取纹理用途
  Usage GetUsage() const noexcept;

  // 是否是立方贴图
  bool IsCubemap() const;
  // 是否多采样点
  bool IsMultiSample() const;
  // 是否是压缩纹理
  bool IsCompressed() const;

  // update cube map
  void SetImage(size_t level, PixelBufferDescriptor&& buffer, const FaceOffsets& face_offsets) const;

  // update 2d map
  void SetImage(size_t level, uint32_t x_offset, uint32_t y_offset,
                uint32_t width, uint32_t height,
                PixelBufferDescriptor&& buffer) const;

  // update 3d map
  void SetImage(size_t level,
                uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
                uint32_t width, uint32_t height, uint32_t depth,
                PixelBufferDescriptor && buffer) const;
  // 生成相应的mipmap
  void GenerateMipmaps();

  // 获取backend的handle
  TextureHandle GetHandle() const {
    return handle_;
  }

  void Destroy() override;
  ~Texture() override= default;
 private:
  // 构造函数
  Texture(const Builder& builder);
 private:
  TextureHandle handle_;
  uint32_t width_ = 1;
  uint32_t height_ = 1;
  uint32_t depth_ = 1;
  InternalFormat format_ = InternalFormat ::RGBA8;
  Sampler target_ = Sampler::SAMPLER_2D;
  uint8_t level_cnt_ = 1;
  uint8_t sample_cnt_ = 1;
  Usage usage_ = Usage::DEFAULT;
  Driver* driver_ {nullptr};
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_TEXTURE_H_
