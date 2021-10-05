//
// Created by Glodxy on 2021/10/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_PIXELBUFFERDESCRIPTOR_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_PIXELBUFFERDESCRIPTOR_H_
#include <cassert>
#include "BufferDescriptor.h"
#include "../include_internal/DriverEnum.h"

namespace our_graph {

/**
 * 专用于Image的Descriptor
 * */
class PixelBufferDescriptor : public BufferDescriptor {
 public:
  PixelBufferDescriptor() = default;

  PixelBufferDescriptor(const void* buffer, size_t size,
                        PixelDataFormat format, PixelDataType type, uint8_t alignment = 1,
                        uint32_t left = 0, uint32_t top = 0, uint32_t stride = 0,
                        Callback callback = nullptr, void* user = nullptr) noexcept
                        : BufferDescriptor(buffer, size, callback, user),
                          left_(left), top_(top), stride_(stride),
                          format_(format), type_(type), alignment_(alignment){

  }

  PixelBufferDescriptor(const void* buffer, size_t size,
                        PixelDataFormat format, PixelDataType type,
                        Callback callback, void* user = nullptr) noexcept
                        : BufferDescriptor(buffer, size, callback, user),
                        stride_(0), format_(format), type_(type), alignment_(1) {

  }

  /**
   * 创建压缩image的descriptor
   * */
  PixelBufferDescriptor(const void* buffer, size_t size,
                        CompressedPixelDataType format, uint32_t image_size,
                        Callback callback = nullptr, void* user = nullptr) noexcept
      : BufferDescriptor(buffer, size, callback, user),
        image_size_(image_size), compressed_format_(format),
        type_(PixelDataType::COMPRESSED), alignment_(1){

  }

  /**
   * 计算该image的字节大小
   * @param format: 每个像素的格式
   * @param type: 每个像素的每个数据的数据类型
   * @param stride: 在像素数组中每行有几个像素，可能大于width
   * @param alignment: 每行像素的对齐字节数
   * @param height: 图片有多少行
   * */
  static constexpr size_t ComputeDataSize(PixelDataFormat format, PixelDataType type,
                                          size_t stride, size_t height, size_t alignment) {
    assert(alignment != 0);

    // 不计算压缩纹理
    if (type == PixelDataType::COMPRESSED) {
      return 0;
    }

    // 每个像素有几个数据
    size_t pixel_data_cnt = 0;
    switch (format) {
      case PixelDataFormat::R:
      case PixelDataFormat::R_INTEGER:
      case PixelDataFormat::DEPTH_COMPONENT:
      case PixelDataFormat::ALPHA:
        pixel_data_cnt = 1;
        break;
      case PixelDataFormat::RG:
      case PixelDataFormat::RG_INTEGER:
      case PixelDataFormat::DEPTH_STENCIL:
        pixel_data_cnt = 2;
        break;
      case PixelDataFormat::RGB:
      case PixelDataFormat::RGB_INTEGER:
        pixel_data_cnt = 3;
        break;
      case PixelDataFormat::UNUSED: // shouldn't happen (used to be rgbm)
      case PixelDataFormat::RGBA:
      case PixelDataFormat::RGBA_INTEGER:
        pixel_data_cnt = 4;
        break;
    }

    // 每个像素的字节数
    size_t bytes_per_pixel = pixel_data_cnt;
    switch (type) {
      case PixelDataType::COMPRESSED: // Impossible -- to squash the IDE warnings
      case PixelDataType::UBYTE:
      case PixelDataType::BYTE:
        // 每个数据1字节，故不变
        break;
      case PixelDataType::USHORT:
      case PixelDataType::SHORT:
      case PixelDataType::HALF:
        // 每个数据2字节
        bytes_per_pixel *= 2;
        break;
      case PixelDataType::UINT:
      case PixelDataType::INT:
      case PixelDataType::FLOAT:
        bytes_per_pixel *= 4;
        break;
      case PixelDataType::UINT_10F_11F_11F_REV:
        // Special case, format must be RGB and uses 4 bytes
        assert(format == PixelDataFormat::RGB);
        bytes_per_pixel = 4;
        break;
      case PixelDataType::UINT_2_10_10_10_REV:
        // Special case, format must be RGBA and uses 4 bytes
        assert(format == PixelDataFormat::RGBA);
        bytes_per_pixel = 4;
        break;
      case PixelDataType::USHORT_565:
        // Special case, format must be RGB and uses 2 bytes
        assert(format == PixelDataFormat::RGB);
        bytes_per_pixel = 2;
        break;
    }
    size_t bytes_per_row = bytes_per_pixel * stride;
    size_t bytes_per_row_aligned = ((bytes_per_row + (alignment - 1)) & (-alignment));
    return bytes_per_row_aligned * height;
  }

 public:
  uint32_t left_ {0};
  uint32_t top_ {0};

  union {
    struct {
      uint32_t stride_;
      PixelDataFormat format_;
    };
    struct {
      uint32_t image_size_;
      CompressedPixelDataType compressed_format_;
    };
  };

  PixelDataType type_ : 4;
  uint8_t alignment_  : 4;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_PIXELBUFFERDESCRIPTOR_H_
