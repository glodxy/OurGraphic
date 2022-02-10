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
template <typename T, size_t Channel = 3>
LinearImage ToLinear(uint32_t w, uint32_t h, uint32_t bpr, const uint8_t* data) {
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
      *d++ = sRGBToLinear(sRGB);
    }
  }
  return result;
}


}  // namespace our_graph::image
#endif //OUR_GRAPHIC_UTILS_IMAGE_COLORTRANSFORM_H_
