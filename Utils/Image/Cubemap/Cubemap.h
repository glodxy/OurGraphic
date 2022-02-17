//
// Created by Glodxy on 2022/2/14.
//

#ifndef OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAP_H_
#define OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAP_H_
#include <algorithm>

#include "Utils/Image/LinearImage.h"
#include "Utils/Math/Math.h"
namespace our_graph::image {
class Cubemap {
 public:
  explicit Cubemap(size_t dim);

  Cubemap(const Cubemap& r) {
    dimension_ = r.dimension_;
    upper_bound_ = r.upper_bound_;
    scale_ = r.scale_;
    for (int i = 0; i < 6; ++i) {
      faces_[i] = r.faces_[i];
    }
  }

  Cubemap& operator=(const Cubemap& r) {
    dimension_ = r.dimension_;
    upper_bound_ = r.upper_bound_;
    scale_ = r.scale_;
    for (int i = 0; i < 6; ++i) {
      faces_[i] = r.faces_[i];
    }
    return *this;
  }

  ~Cubemap();

  enum class Face : uint8_t {
    PX = 0, // right
    NX, // left
    PY, // top
    NY, // bot
    PZ, // front
    NZ // back
  };

  using Texel = math::Vec3;

  /**
   * 重置cubemap的大小，该操作会release所有持有的图片
   * */
  void ResetDimension(size_t dim);

  void SetImageForFace(Face face, const LinearImage& image);

  inline const LinearImage& GetImageForFace(Face face) const;
  inline LinearImage& GetImageForFace(Face face);

  /**
   * 获取（x，y）位置的像素中心所对应的纹理坐标
   * */
  static inline math::Vec2 GetCenter(size_t x, size_t y);

  /**
   * 获取从相机位置（中心）到指定face上的执行像素（x，y）中心的方向向量
   * */
  inline math::Vec3 GetDirectionFor(Face face, size_t x, size_t y);
  // 同上,但直接获取参数位置
  inline math::Vec3 GetDirectionFor(Face face, float x, float y);

  /**
   * 采样direction方向上的纹理
   * */
  inline Texel SampleAt(const math::Vec3& direction);
  inline Texel FilterAt(const math::Vec3& direction);

  // 通过双线性过滤对坐标进行采样
  static Texel FilterAt(const LinearImage& image, float x, float y);
  // 通过双线性过滤对目标像素的中心进行采样
  static Texel FilterAtCenter(const LinearImage& image, size_t x, size_t y);
  /**
   * 通过三重线性过滤来对坐标进行采样
   * @param c0，c1：不同层级的cubemap
   * */
  static Texel TrilinearFilterAt(const Cubemap& c0, const Cubemap& c1, float lerp,
                                const math::Vec3& direction);

  // 读取该位置的数据，转为texel
  inline static const Texel & ReadAt(const void* data) {
    return *static_cast<const Texel*>(data);
  }
  // 将texel写入该位置
  inline static void WriteAt(const Texel& texel, void* data) {
    *static_cast<Texel*>(data) = texel;
  }

  // 获取dimension
  size_t GetDimension() const;

  // 让整个cubemap无缝化
  //! 方便之后的采样，防止采样到缝隙
  void MakeSeamless();


  /**
   * 该结构体用于定位某个面上的具体texel
   * */
  struct Address {
    Face face;
    float s = 0;
    float t = 0;
  };
  //! 根据direction获取对应的address（转换为address）
  static Address GetAddressFor(const math::Vec3& direction);

 private:
  size_t dimension_ = 0;  // 即每个面的宽(因为正方形贴图，宽=高）
  float scale_ = 1.0f;
  float upper_bound_ = 0;
  LinearImage faces_[6];
};

inline const LinearImage& Cubemap::GetImageForFace(Face face) const {
  return faces_[(uint8_t)face];
}

inline LinearImage& Cubemap::GetImageForFace(Face face) {
  return faces_[(uint8_t)face];
}

inline math::Vec2 Cubemap::GetCenter(size_t x, size_t y) {
  return {x + .5f, y + .5f};
}

inline math::Vec3 Cubemap::GetDirectionFor(Face face, size_t x, size_t y) {
  return GetDirectionFor(face, x + .5f, y + .5f);
}

inline math::Vec3 Cubemap::GetDirectionFor(Face face, float x, float y) {
  // 将图片的[0, dim]区间映射到[-1, 1]
  // 此处的scale = 2 / dim
  float cx = (x * scale_) - 1;
  float cy = 1 - (y*scale_);

  // 根据面的位置来获取对应的坐标
  math::Vec3 dir;
  const float l = std::sqrt(cx * cx + cy * cy + 1);
  switch (face) {
    case Face::PX: dir = {1, -cy, cx}; break;
    case Face::NX: dir = {-1, -cy, -cx}; break;
    case Face::PY: dir = {cx, 1, -cy}; break;
    case Face::NY: dir = {cx, -1, cy}; break;
    case Face::PZ: dir = {cx, -cy, 1}; break;
    case Face::NZ: dir = {-cx, -cy, -1}; break;
  }
  // 返回单位向量
  return dir / l;
}

inline Cubemap::Texel Cubemap::SampleAt(const math::Vec3& direction) {
  Cubemap::Address address(GetAddressFor(direction));
  // 计算坐标防止越界
  const size_t x = std::min(size_t(address.s * dimension_), dimension_ - 1);
  const size_t y = std::min(size_t(address.t * dimension_), dimension_ - 1);
  return ReadAt(GetImageForFace(address.face).GetPixel(y, x));
}

inline Cubemap::Texel Cubemap::FilterAt(const math::Vec3& direction) {
  Cubemap::Address address(GetAddressFor(direction));
  address.s = std::min(address.s * dimension_, upper_bound_);
  address.t = std::min(address.t * dimension_, upper_bound_);
  return FilterAt(GetImageForFace(address.face), address.s, address.t);
}

}  // namespace our_graph::image
#endif //OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAP_H_
