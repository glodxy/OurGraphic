//
// Created by Glodxy on 2021/10/27.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_TEXTURE_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_TEXTURE_H_
#include "Backend/include/DriverEnum.h"
#include "Backend/include/PixelBufferDescriptor.h"
#include "Framework/Resource/base/BuilderBase.h"
namespace our_graph {

class Texture {
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

  static bool IsTextureFormatSupported(InternalFormat format) noexcept;

  static bool IsTextureSwizzleSupported() noexcept;

  static size_t ComputerTextureDataSize(Format format, Type type,
                                        size_t stride, size_t height, size_t alignment) noexcept;

  class Builder : public BuilderBase<BuilderDetails> {
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

  size_t GetWidth(size_t level = BASE_LEVEL) const noexcept;

  size_t GetHeight(size_t level = BASE_LEVEL) const noexcept;

  size_t GetLevels() const noexcept;

  Sampler GetSamplerType() const noexcept;

  InternalFormat GetFormat() const noexcept;


  void SetImage(size_t level, PixelBufferDescriptor&& buffer) const;
  void SetImage(size_t level, uint32_t x_offset, uint32_t y_offset,
                uint32_t width, uint32_t height,
                PixelBufferDescriptor&& buffer) const;

  void SetImage(size_t level,
                uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
                uint32_t width, uint32_t height, uint32_t depth,
                PixelBufferDescriptor && buffer) const;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_TEXTURE_H_
