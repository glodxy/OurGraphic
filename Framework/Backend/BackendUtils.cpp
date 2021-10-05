//
// Created by Glodxy on 2021/10/5.
//
#include "BackendUtils.h"
#include "DataReshaper.h"
namespace our_graph {
using utils::DataReshaper;

bool BackendUtil::Reshape(const PixelBufferDescriptor &src_data, PixelBufferDescriptor &reshape_data) {
  if (!(src_data.format_ == PixelDataFormat::RGB ||
        src_data.format_ == PixelDataFormat::RGB_INTEGER)) {
    return false;
  }

  const auto free_func = [](void* buffer, size_t size, void* user) {
    free(buffer);
  };
  // 将三个数据的大小扩展到四个数据
  const size_t reshape_size = 4 * src_data.size_ / 3;
  const PixelDataFormat reshape_format =
      src_data.format_ == PixelDataFormat::RGB ?
      PixelDataFormat::RGBA : PixelDataFormat::RGBA_INTEGER;

  switch (src_data.type_) {
    case PixelDataType::BYTE:
    case PixelDataType::UBYTE: {
      uint8_t * bytes = (uint8_t*) malloc(reshape_size);
      DataReshaper::reshape<uint8_t, 3, 4>(bytes, src_data.buffer_, src_data.size_);
      PixelBufferDescriptor pbd(bytes, reshape_size, reshape_format, src_data.type_,
                                src_data.alignment_, src_data.left_, src_data.top_,
                                src_data.stride_, free_func);
      reshape_data = std::move(pbd);
      return true;
    }

    case PixelDataType::SHORT:
    case PixelDataType::USHORT:
    case PixelDataType::HALF: {
      uint8_t * bytes = (uint8_t*) malloc(reshape_size);
      DataReshaper::reshape<uint16_t, 3, 4>(bytes, src_data.buffer_, src_data.size_);
      PixelBufferDescriptor pbd(bytes, reshape_size, reshape_format, src_data.type_,
                                src_data.alignment_, src_data.left_, src_data.top_,
                                src_data.stride_, free_func);
      reshape_data = std::move(pbd);
      return true;
    }
    case PixelDataType::INT:
    case PixelDataType::UINT: {
      uint8_t * bytes = (uint8_t*) malloc(reshape_size);
      DataReshaper::reshape<uint32_t, 3, 4>(bytes, src_data.buffer_, src_data.size_);
      PixelBufferDescriptor pbd(bytes, reshape_size, reshape_format, src_data.type_,
                                src_data.alignment_, src_data.left_, src_data.top_,
                                src_data.stride_, free_func);
      reshape_data = std::move(pbd);
      return true;
    }
    case PixelDataType::FLOAT: {
      uint8_t * bytes = (uint8_t*) malloc(reshape_size);
      DataReshaper::reshape<float, 3, 4>(bytes, src_data.buffer_, src_data.size_);
      PixelBufferDescriptor pbd(bytes, reshape_size, reshape_format, src_data.type_,
                                src_data.alignment_, src_data.left_, src_data.top_,
                                src_data.stride_, free_func);
      reshape_data = std::move(pbd);
      return true;
    }
    default:
      return false;
  }
}


size_t BackendUtil::GetFormatSize(TextureFormat format) {
  switch (format) {
    // 8-bits per element
    case TextureFormat::R8:
    case TextureFormat::R8_SNORM:
    case TextureFormat::R8UI:
    case TextureFormat::R8I:
    case TextureFormat::STENCIL8:
      return 1;

      // 16-bits per element
    case TextureFormat::R16F:
    case TextureFormat::R16UI:
    case TextureFormat::R16I:
    case TextureFormat::RG8:
    case TextureFormat::RG8_SNORM:
    case TextureFormat::RG8UI:
    case TextureFormat::RG8I:
    case TextureFormat::RGB565:
    case TextureFormat::RGB5_A1:
    case TextureFormat::RGBA4:
    case TextureFormat::DEPTH16:
      return 2;

      // 24-bits per element
    case TextureFormat::RGB8:
    case TextureFormat::SRGB8:
    case TextureFormat::RGB8_SNORM:
    case TextureFormat::RGB8UI:
    case TextureFormat::RGB8I:
    case TextureFormat::DEPTH24:
      return 3;

      // 32-bits per element
    case TextureFormat::R32F:
    case TextureFormat::R32UI:
    case TextureFormat::R32I:
    case TextureFormat::RG16F:
    case TextureFormat::RG16UI:
    case TextureFormat::RG16I:
    case TextureFormat::R11F_G11F_B10F:
    case TextureFormat::RGB9_E5:
    case TextureFormat::RGBA8:
    case TextureFormat::SRGB8_A8:
    case TextureFormat::RGBA8_SNORM:
    case TextureFormat::RGB10_A2:
    case TextureFormat::RGBA8UI:
    case TextureFormat::RGBA8I:
    case TextureFormat::DEPTH32F:
    case TextureFormat::DEPTH24_STENCIL8:
    case TextureFormat::DEPTH32F_STENCIL8:
      return 4;

      // 48-bits per element
    case TextureFormat::RGB16F:
    case TextureFormat::RGB16UI:
    case TextureFormat::RGB16I:
      return 6;

      // 64-bits per element
    case TextureFormat::RG32F:
    case TextureFormat::RG32UI:
    case TextureFormat::RG32I:
    case TextureFormat::RGBA16F:
    case TextureFormat::RGBA16UI:
    case TextureFormat::RGBA16I:
      return 8;

      // 96-bits per element
    case TextureFormat::RGB32F:
    case TextureFormat::RGB32UI:
    case TextureFormat::RGB32I:
      return 12;

      // 128-bits per element
    case TextureFormat::RGBA32F:
    case TextureFormat::RGBA32UI:
    case TextureFormat::RGBA32I:
      return 16;

      // Compressed formats ---------------------------------------------------------------------

    case TextureFormat::EAC_RG11:
    case TextureFormat::EAC_RG11_SIGNED:
    case TextureFormat::ETC2_EAC_RGBA8:
    case TextureFormat::ETC2_EAC_SRGBA8:
      return 16;

    case TextureFormat::EAC_R11:
    case TextureFormat::EAC_R11_SIGNED:
    case TextureFormat::ETC2_RGB8:
    case TextureFormat::ETC2_SRGB8:
    case TextureFormat::ETC2_RGB8_A1:
    case TextureFormat::ETC2_SRGB8_A1:
      return 8;

    case TextureFormat::DXT1_RGB:
    case TextureFormat::DXT1_RGBA:
    case TextureFormat::DXT1_SRGB:
    case TextureFormat::DXT1_SRGBA:
      return 8;

    case TextureFormat::DXT3_RGBA:
    case TextureFormat::DXT3_SRGBA:
    case TextureFormat::DXT5_RGBA:
    case TextureFormat::DXT5_SRGBA:
      return 16;

      // The block size for ASTC compression is always 16 bytes.
    case TextureFormat::RGBA_ASTC_4x4:
    case TextureFormat::RGBA_ASTC_5x4:
    case TextureFormat::RGBA_ASTC_5x5:
    case TextureFormat::RGBA_ASTC_6x5:
    case TextureFormat::RGBA_ASTC_6x6:
    case TextureFormat::RGBA_ASTC_8x5:
    case TextureFormat::RGBA_ASTC_8x6:
    case TextureFormat::RGBA_ASTC_8x8:
    case TextureFormat::RGBA_ASTC_10x5:
    case TextureFormat::RGBA_ASTC_10x6:
    case TextureFormat::RGBA_ASTC_10x8:
    case TextureFormat::RGBA_ASTC_10x10:
    case TextureFormat::RGBA_ASTC_12x10:
    case TextureFormat::RGBA_ASTC_12x12:
    case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
    case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
    case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
    case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
    case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
    case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
    case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
    case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
    case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
    case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
    case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
    case TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
    case TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
    case TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
      return 16;

    default:
      return 0;
  }
}

}  // namespace our_graph