//
// Created by Glodxy on 2022/2/10.
//

#ifndef OUR_GRAPHIC_UTILS_IMAGE_COLORTRANSFORM_H_
#define OUR_GRAPHIC_UTILS_IMAGE_COLORTRANSFORM_H_
#include "LinearImage.h"
#include "Utils/Math/Math.h"
namespace our_graph::image {
/**
 * 该部分用于颜色空间的转换
 * @todo:目前仅用于转换为linear
 * */


/*----------------Linear => gray-----------------*/
template <typename T>
std::unique_ptr<uint8_t[]> FromLinearToGrayscale(const LinearImage& image) {
  const size_t w = image.GetWidth();
  const size_t h = image.GetHeight();
  assert(image.GetChannels() == 1);
  std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * sizeof(T)]);
  T* d = reinterpret_cast<T*>(dst.get());
  for (size_t y = 0; y < h; ++y) {
    float const* p = image.GetPixel(0, y);
    for (size_t x = 0; x < w; ++x, ++p, ++d) {
      const float gray =  glm::clamp(*p, .0f, 1.0f) * std::numeric_limits<T>::max() + 0.5f;
      d[0] = T(gray);
    }
  }
  return dst;
}

/*--------Linear => sRGB----------------*/

template <typename T>
inline math::Vec3 LinearTosRGB(const T& linear) {
  constexpr float a = 0.055f;
  constexpr float a1 = 1.055f;
  constexpr float p = 1 / 2.4f;
  math::Vec3 sRGB;
  for (size_t i=0 ; i<3 ; i++) {
    if (linear[i] <= 0.0031308f) {
      sRGB[i] = linear[i] * 12.92f;
    } else {
      sRGB[i] = a1 * std::pow(linear[i], p) - a;
    }
  }
  return sRGB;
}

inline float LinearTosRGB(float linear) {
  if (linear <= 0.0031308f) {
    return linear * 12.92f;
  } else {
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 1 / 2.4f;
    return a1 * std::pow(linear, p) - a;
  }
}

/**
 * 只会取前三个通道进行颜色空间的转换
 * */
template<typename T, int N = 3>
std::unique_ptr<uint8_t[]> FromLinearTosRGB(const LinearImage& image) {
  const size_t w = image.GetWidth();
  const size_t h = image.GetHeight();
  const size_t nchan = image.GetChannels();
  assert(nchan >= N);
  std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * N * sizeof(T)]);
  T* d = reinterpret_cast<T*>(dst.get());
  for (size_t y = 0; y < h; ++y) {
    float const* p = image.GetPixel(y, 0);
    for (size_t x = 0; x < w; ++x, p += nchan, d += N) {
      for (int n = 0; n < N; n++) {
        float source = n < 3 ? LinearTosRGB(p[n]) : p[n];
        float target =  glm::clamp(source, .0f, 1.0f) * std::numeric_limits<T>::max() + 0.5f;
        d[n] = T(target);
      }
    }
  }
  return dst;
}

/*---------------Linear => RGB -------------------*/
template<typename T, int N = 3>
std::unique_ptr<uint8_t[]> FromLinearToRGB(const LinearImage& image) {
  size_t w = image.GetWidth();
  size_t h = image.GetHeight();
  size_t channels = image.GetChannels();
  assert(channels >= N);
  std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * N * sizeof(T)]);
  T* d = reinterpret_cast<T*>(dst.get());
  for (size_t y = 0; y < h; ++y) {
    float const* p = image.GetPixel(y, 0);
    for (size_t x = 0; x < w; ++x, p += channels, d += N) {
      for (int n = 0; n < channels; n++) {
        float target =  glm::clamp(p[n], 0.f, 1.f) * std::numeric_limits<T>::max() + 0.5f;
        d[n] = T(target);
      }
    }
  }
  return dst;
}

/*-----------------sRGB => Linear ---------------*/
template<typename T>
T sRGBToLinear(const T& sRGB);

template<>
inline math::Vec3 sRGBToLinear(const math::Vec3 & sRGB) {
  constexpr float a = 0.055f;
  constexpr float a1 = 1.055f;
  constexpr float p = 2.4f;
  math::Vec3 linear;
  for (size_t i=0 ; i<3 ; i++) {
    if (sRGB[i] <= 0.04045f) {
      linear[i] = sRGB[i] * (1.0f / 12.92f);
    } else {
      linear[i] = std::pow((sRGB[i] + a) / a1, p);
    }
  }
  return linear;
}

template<>
inline math::Vec4 sRGBToLinear(const math::Vec4 & sRGB) {
  constexpr float a = 0.055f;
  constexpr float a1 = 1.055f;
  constexpr float p = 2.4f;
  math::Vec4 linear;
  for (size_t i=0 ; i<3 ; i++) {
    if (sRGB[i] <= 0.04045f) {
      linear[i] = sRGB[i] * (1.0f / 12.92f);
    } else {
      linear[i] = std::pow((sRGB[i] + a) / a1, p);
    }
  }
  linear[3] = sRGB[3];
  return linear;
}
/*--------------------------------------------*/


/**
 * 将数据转换为LinearImage
 * @param bpr:bytes per row, 每行的字节数
 * @param data: 原始数据
 *
 * @tparam T:数据类型
 * @tparam Channel:通道数
 * */
template <typename T = uint8_t, size_t Channel = 3>
LinearImage FromSRGBToLinear(uint32_t w, uint32_t h, uint32_t bpr, const uint8_t* data) {
  using PixelVec = glm::vec<Channel, float>;
  LinearImage result(w, h, Channel);
  // 得到像素的指针d
  PixelVec * d = reinterpret_cast<PixelVec *>(result.GetPixel(0, 0));

  for (uint32_t y = 0; y < h; ++y) {
    // 得到该行的起始指针
    const T* p = reinterpret_cast<const T*>(data + y * bpr);
    // 每次p移动一个像素的距离
    for (uint32_t x = 0; x < w; ++x, p+= Channel) {
      PixelVec sRGB;
      for (int i = 0; i < Channel; ++i) {
        sRGB[i] = p[i];
      }
      sRGB /= std::numeric_limits<T>::max();
      sRGB = sRGBToLinear(sRGB);
      *d++ = sRGB;
    }
  }
  return result;
}

template <typename T = uint8_t, size_t Channel = 3>
LinearImage FromRGBToLinear(uint32_t w, uint32_t h, uint32_t bpr, const uint8_t* data) {
  using PixelVec = glm::vec<Channel, float>;
  LinearImage result(w, h, Channel);
  // 得到像素的指针d
  PixelVec * d = reinterpret_cast<PixelVec *>(result.GetPixel(0, 0));

  for (uint32_t y = 0; y < h; ++y) {
    // 得到该行的起始指针
    const T* p = reinterpret_cast<const T*>(data + y * bpr);
    // 每次p移动一个像素的距离
    for (uint32_t x = 0; x < w; ++x, p+= Channel) {
      PixelVec RGB;
      for (int i = 0; i < Channel; ++i) {
        RGB[i] = p[i];
      }
      RGB /= std::numeric_limits<T>::max();
      *d++ = RGB;
    }
  }
  return result;
}


}  // namespace our_graph::image
#endif //OUR_GRAPHIC_UTILS_IMAGE_COLORTRANSFORM_H_
