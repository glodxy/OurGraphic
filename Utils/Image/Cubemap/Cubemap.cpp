//
// Created by Glodxy on 2022/2/14.
//

#include "Cubemap.h"

namespace our_graph::image {

Cubemap::Cubemap(size_t dim) {
  ResetDimension(dim);
}

Cubemap::~Cubemap() = default;

size_t Cubemap::GetDimension() const {
  return dimension_;
}

void Cubemap::ResetDimension(size_t dim) {
  dimension_ = dim;
  scale_ = 2.f / dim;
  upper_bound_ = std::nextafter((float)dimension_, .0f);
  for (auto& face : faces_) {
    face.Reset();
  }
}

void Cubemap::SetImageForFace(Face face, const LinearImage &image) {
  faces_[(uint8_t)face] = image;
}


Cubemap::Address Cubemap::GetAddressFor(const math::Vec3 &direction) {
  Cubemap::Address addr;
  float sc, tc, ma;
  const float rx = std::abs(direction.x);
  const float ry = std::abs(direction.y);
  const float rz = std::abs(direction.z);
  if (rx >= ry && rx >= rz) {
    ma = 1.0f / rx;
    if (direction.x >= 0) {
      addr.face = Face::PX;
      sc = direction.z;
      tc = direction.y;
    } else {
      addr.face = Face::NX;
      sc = -direction.z;
      tc = direction.y;
    }
  } else if (ry >= rx && ry >= rz) {
    ma = 1.0f / ry;
    if (direction.y >= 0) {
      addr.face = Face::PY;
      sc =  direction.x;
      tc =  direction.z;
    } else {
      addr.face = Face::NY;
      sc =  direction.x;
      tc = -direction.z;
    }
  } else {
    ma = 1.0f / rz;
    if (direction.z >= 0) {
      addr.face = Face::PZ;
      sc = -direction.x;
      tc = direction.y;
    } else {
      addr.face = Face::NZ;
      sc = direction.x;
      tc = direction.y;
    }
  }
  // 因为ma相当于 1/（1/scale）= scale, 而其他数值为[-1, 1]/scale，所以此处相当于做了个逆变换
  // 此处乘ma将坐标转换至[-1, 1]
  // 然后映射至[0, 1]
  addr.s = (sc * ma + 1.0f) * 0.5f;
  addr.t = (tc * ma + 1.0f) * 0.5f;
  return addr;
}


void Cubemap::MakeSeamless() {
  size_t D = dimension_;

  const size_t bpr = GetImageForFace(Face::PX).GetAbsoluteBytesPerRow();
  const size_t bpp = GetImageForFace(Face::PX).GetBytesPerPixel();

  auto GetTexel = [](LinearImage& image, ssize_t x, ssize_t y) -> Texel* {
    return (Texel*)((uint8_t*)image.GetData() + x * image.GetBytesPerPixel() + y * image.GetAbsoluteBytesPerRow());
  };

  //! 该函数用于将指定位置之后的一维像素从src拷贝至dst
  //! 用于处理相邻边的采样
  auto stitch = [ & ](
      Face face_dst, ssize_t xdst, ssize_t ydst, size_t inc_dst,
      Face face_src, size_t xsrc, size_t ysrc, ssize_t inc_src) {
    LinearImage& dst_img = GetImageForFace(face_dst);
    LinearImage& src_img = GetImageForFace(face_src);
    Texel* dst = GetTexel(dst_img, xdst, ydst);
    Texel* src = GetTexel(src_img, xsrc, ysrc);
    for (size_t i = 0; i < D; ++i) {
      *dst = *src;
      dst = (Texel*)((uint8_t*)dst + inc_dst);
      src = (Texel*)((uint8_t*)src + inc_src);
    }
  };

  //! 该函数用于处理相接角的采样
  auto corners = [ & ](Face face) {
    size_t L = D - 1;
    LinearImage& image = GetImageForFace(face);
    *GetTexel(image,  -1,  -1) = (*GetTexel(image, 0, 0) + *GetTexel(image,  -1,  0) + *GetTexel(image, 0,    -1)) / 3.f;
    *GetTexel(image, L+1,  -1) = (*GetTexel(image, L, 0) + *GetTexel(image,   L, -1) + *GetTexel(image, L+1,   0)) / 3.f;
    *GetTexel(image,  -1, L+1) = (*GetTexel(image, 0, L) + *GetTexel(image,  -1,  L) + *GetTexel(image, 0,   L+1)) / 3.f;
    *GetTexel(image, L+1, L+1) = (*GetTexel(image, L, L) + *GetTexel(image, L+1,  L) + *GetTexel(image, L+1,   L)) / 3.f;
  };

  // todo
  //PY
  stitch( Face::PY, -1,  0,  bpr, Face::NX, 0, 0, bpp);      // left
  stitch( Face::PY,  0, -1,  bpp, Face::NZ,  D-1,  0, -bpp);      // bot
  stitch( Face::PY,  D,  0,  bpr, Face::PX,  D-1,  0,   -bpp);      // right
  stitch( Face::PY,  0,  D,  bpp, Face::PZ,  0,    0, bpp);      // top
  corners(Face::PY);

  // NX
  stitch( Face::NX, -1,  0,  bpr, Face::PZ,  D-1,  0,    bpr);      // left
  stitch( Face::NX,  0, -1,  bpp, Face::PY,  0,  0, bpr);      // top
  stitch( Face::NX,  D,  0,  bpr, Face::NZ,  0,  0,    bpr);      // right
  stitch( Face::NX,  0,  D,  bpp, Face::NY,  0,    D-1, -bpr);      // bottom
  corners(Face::NX);

  // PZ
  stitch( Face::PZ, -1,  0,  bpr, Face::PX,  D-1,  0,    bpr);      // left
  stitch( Face::PZ,  0, -1,  bpp, Face::PY,  D-1,    0,  -bpp);      // top
  stitch( Face::PZ,  D,  0,  bpr, Face::NX,  0,    0,    bpr);      // right
  stitch( Face::PZ,  0,  D,  bpp, Face::NY,  D-1,    D-1,    -bpp);      // bottom
  corners(Face::PZ);

  // PX
  stitch( Face::PX, -1,  0,  bpr, Face::NZ,  D-1,  0,    bpr);      // left
  stitch( Face::PX,  0, -1,  bpp, Face::PY,  D-1,  D-1, -bpr);      // top
  stitch( Face::PX,  D,  0,  bpr, Face::PZ,  0,    0,    bpr);      // right
  stitch( Face::PX,  0,  D,  bpp, Face::NY,  D-1,  0,    bpr);      // bottom
  corners(Face::PX);

  // NZ
  stitch( Face::NZ, -1,  0,  bpr, Face::NX,  D-1,  0,    bpr);      // left
  stitch( Face::NZ,  0, -1,  bpp, Face::PY,  0,  D-1,   bpp);      // top
  stitch( Face::NZ,  D,  0,  bpr, Face::PX,  0,    0,    bpr);      // right
  stitch( Face::NZ,  0,  D,  bpp, Face::NY,  0,  0, bpp);      // bottom
  corners(Face::NZ);

  // NY
  stitch( Face::NY, -1,  0,  bpr, Face::NX,  D-1,  D-1, -bpp);      // left
  stitch( Face::NY,  0, -1,  bpp, Face::PZ,  0,    D-1,  bpp);      // top
  stitch( Face::NY,  D,  0,  bpr, Face::PX,  0,    D-1,  bpp);      // right
  stitch( Face::NY,  0,  D,  bpp, Face::NZ,  D-1,  D-1, -bpp);      // bottom
  corners(Face::NY);
}

Cubemap::Texel Cubemap::FilterAt(const LinearImage &image, float x, float y) {
  const size_t x0 = size_t(x);
  const size_t y0 = size_t(y);
  size_t x1 = x0 + 1;
  size_t y1 = y0 + 1;

  const float u = float(x - x0);
  const float v = float(y - y0);
  const float one_minus_u = 1 - u;
  const float one_minus_v = 1 - v;
  const Texel& c0 = ReadAt(image.GetPixel(y0, x0));
  const Texel& c1 = ReadAt(image.GetPixel(y0, x1));
  const Texel& c2 = ReadAt(image.GetPixel(y1, x0));
  const Texel& c3 = ReadAt(image.GetPixel(y1, x1));
  // 按照比重赋值
  return (one_minus_u*one_minus_v)*c0 + (u*one_minus_v)*c1 + (one_minus_u*v)*c2 + (u*v)*c3;
}

Cubemap::Texel Cubemap::FilterAtCenter(const LinearImage &image, size_t x, size_t y) {
  size_t x1 = x + 1;
  size_t y1 = y + 1;
  const Texel& c0 = ReadAt(image.GetPixel(y, x));
  const Texel& c1 = ReadAt(image.GetPixel(y, x1));
  const Texel& c2 = ReadAt(image.GetPixel(y1, x));
  const Texel& c3 = ReadAt(image.GetPixel(y1, x1));
  return (c0 + c1 + c2 + c3) * 0.25f;
}


Cubemap::Texel Cubemap::TrilinearFilterAt(const Cubemap &c0, const Cubemap &c1, float lerp, const math::Vec3 &direction) {
  Cubemap::Address addr(GetAddressFor(direction));
  // 对两个面进行采样，并进行插值
  const LinearImage& i0 = c0.GetImageForFace(addr.face);
  const LinearImage& i1 = c1.GetImageForFace(addr.face);
  float x0 = std::min(addr.s * c0.dimension_, c0.upper_bound_);
  float y0 = std::min(addr.t * c0.dimension_, c0.upper_bound_);
  float x1 = std::min(addr.s * c1.dimension_, c1.upper_bound_);
  float y1 = std::min(addr.t * c1.dimension_, c1.upper_bound_);
  math::Vec3 r0 = FilterAt(i0, x0, y0);
  r0 += lerp * (FilterAt(i1, x1, y1) - r0);
  return r0;
}
}  // namespace our_graph::image