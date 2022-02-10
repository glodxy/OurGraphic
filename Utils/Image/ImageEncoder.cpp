//
// Created by Glodxy on 2022/2/10.
//

#include "ImageEncoder.h"

#include <ostream>
#include "png.h"
#include "Utils/OGLogging.h"
namespace our_graph::image {

class PNGEncoder : public ImageEncoder::BaseEncoder {
 public:

  enum class PixelFormat {
    sRGB,
    LINEAR_RGB
  };
  // 创建一个png encoder， format为输出格式
  static PNGEncoder* Create(std::ostream& out, PixelFormat format = PixelFormat::sRGB);

  PNGEncoder(const PNGEncoder&) = delete;
  PNGEncoder& operator=(const PNGEncoder&) = delete;
 protected:
  PNGEncoder(std::ostream& stream, PixelFormat format);
  ~PNGEncoder() override;

  //! 初始化，在此处绑定回调
  void Init();

  bool Encode(const LinearImage &image) override;

  // libpng的回调函数
  static void cb_error(png_structp png, png_const_charp error);
  static void cb_stream(png_structp png, png_bytep buffer, png_size_t size);

  // 在此处写入流
  void Write(void* buffer, size_t size);
  // 在此处抛出错误
  void Error();

  // 选择颜色的类别
  int SelectColorType(const LinearImage& linear_image);
  // 根据颜色类别获取通道数
  uint32_t GetChannelCnt(int color_type);
 private:
  png_structp png_ = nullptr;  // png的结构体
  png_infop info_ = nullptr;  // png的信息

  std::ostream& out_stream_;
  std::streampos out_pos_;

  PixelFormat format_;  // 要写出的格式
};

PNGEncoder * PNGEncoder::Create(std::ostream &out, PixelFormat format) {
  PNGEncoder* encoder = new PNGEncoder(out, format);
  encoder->Init();
  return encoder;
}

PNGEncoder::PNGEncoder(std::ostream &stream, PixelFormat format)
  : png_(png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr)),
  out_stream_(stream), out_pos_(stream.tellp()), format_(format) {}

PNGEncoder::~PNGEncoder() noexcept {
  png_destroy_write_struct(&png_, &info_);
}

void PNGEncoder::Init() {
  png_set_error_fn(png_, this, cb_error, nullptr);
  png_set_write_fn(png_, this, cb_stream, nullptr);
}

int PNGEncoder::SelectColorType(const LinearImage &linear_image) {
  size_t channels = linear_image.GetChannels();
  switch (channels) {
    case 1: return PNG_COLOR_TYPE_GRAY;
    case 3: {
      switch (format_) {
        default: return PNG_COLOR_TYPE_RGBA;
      }
    }
    case 4: return PNG_COLOR_TYPE_RGBA;
    default: {
      LOG_ERROR("PNGEncoder", "Cannot find true color type!");
      return PNG_COLOR_TYPE_RGB;
    }
  }
}

uint32_t PNGEncoder::GetChannelCnt(int color_type) {
  switch (format_) {
    default: {
      switch (color_type) {
        case PNG_COLOR_TYPE_GRAY: return 1;
        case PNG_COLOR_TYPE_RGB: return 3;
        case PNG_COLOR_TYPE_RGBA: return 4;
      }
      return 3;
    }
  }
}

bool PNGEncoder::Encode(const LinearImage &image) {
  uint32_t channels = image.GetChannels();
  switch (format_) {
    default: {
      if (channels != 1 && channels != 3 && channels != 4) {
        LOG_ERROR("PNGEncoder", "Cannot Encode for channels:[{}]", channels);
        return false;
      }
      break;
    }
  }

  try {
    info_ = png_create_info_struct(png_);

    uint32_t width = image.GetWidth();
    uint32_t height = image.GetHeight();
    int color_type = SelectColorType(image);

    // 此处设置信息头
    png_set_IHDR(png_, info_, width, height,
                 8, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    if (format_ == PixelFormat::LINEAR_RGB) {
      png_set_gAMA(png_, info_, 1.0);
    } else {
      png_set_sRGB_gAMA_and_cHRM(png_, info_, PNG_sRGB_INTENT_PERCEPTUAL);
    }

    // 将信息写入png
    png_write_info(png_, info_);

    // 创建每行的像素数据
    std::unique_ptr<png_bytep[]> row_pointers(new png_bytep[height]);
    std::unique_ptr<uint8_t[]> data;

    uint32_t dst_channels;
    // 此处将数据写入data
    if (channels == 1) {
      dst_channels = 1;
      data = fromLinearToGrayscale<uint8_t>(image);
    } else {
      dst_channels = GetChannelCnt(color_type);
      switch (format_) {
        case PixelFormat::sRGB:
          if (dst_channels == 4) {
            data = fromLinearTosRGB<uint8_t, 4>(image);
          } else {
            data = fromLinearTosRGB<uint8_t, 3>(image);
          }
          break;
        case PixelFormat::LINEAR_RGB:
          if (dst_channels == 4) {
            data = fromLinearToRGB<uint8_t, 4>(image);
          } else {
            data = fromLinearToRGB<uint8_t, 3>(image);
          }
          break;
      }
    }

    // 此处将数据写入每行像素
    for (size_t y = 0; y < height; y++) {
      row_pointers[y] = reinterpret_cast<png_bytep>
      (&data[y * width * dst_channels * sizeof(uint8_t)]);
    }

    png_write_image(png_, row_pointers.get());
    png_write_end(png_, info_);
    out_stream_.flush();
  } catch (std::runtime_error& e) {
    std::cerr << "Runtime error while encoding PNG: " << e.what() << std::endl;
    out_stream_.seekp(out_pos_);
    return false;
  }
  return true;
}

void PNGEncoder::cb_stream(png_structp png, png_bytep buffer, png_size_t size) {
  PNGEncoder* encoder = static_cast<PNGEncoder*>(png_get_io_ptr(png));
  encoder->Write(buffer, size);
}

void PNGEncoder::Write(void* buffer, size_t size) {
  out_stream_.write(static_cast<char *>(buffer), size);
  if (!out_stream_.good()) {
    throw std::runtime_error("Problem with the PNG stream.");
  }
}

void PNGEncoder::cb_error(png_structp png, png_const_charp) {
  PNGEncoder* encoder = static_cast<PNGEncoder*>(png_get_error_ptr(png));
  encoder->Error();
}

void PNGEncoder::Error() {
  throw std::runtime_error("Error while encoding PNG stream.");
}

//! ImageEncoder-------------------------

bool ImageEncoder::Encode(std::ostream &out, Format format, const LinearImage &image) {
  std::unique_ptr<BaseEncoder> encoder;
  switch (format) {
    case Format::PNG: {
      encoder.reset(PNGEncoder::Create(out));
      break;
    }
  }

  return encoder->Encode(image);
}


}  // namespace our_graph::image