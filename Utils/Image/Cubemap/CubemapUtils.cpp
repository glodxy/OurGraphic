//
// Created by Glodxy on 2022/2/14.
//

#include "CubemapUtils.h"
#include "Utils/OGLogging.h"
#include <vector>

namespace our_graph::image {
Cubemap CubemapUtils::CreateCubemap(LinearImage& image, size_t dim) {
  Cubemap cubemap(dim);
  LinearImage limage = CreateCubemapImage(dim);
  for (size_t i = 0; i < 6; ++i) {
    BindFaceFromWholeImage(cubemap, (Cubemap::Face)i, limage);
  }
  std::swap(image, limage);
  return cubemap;
}

math::Vec2 CubemapUtils::Hammersley(uint32_t i, float iN) {
  constexpr float tof = 0.5f / 0x80000000U;
  uint32_t bits = i;
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return { i * iN, bits * tof };
}

LinearImage CubemapUtils::CreateCubemapImage(size_t dim) {
  size_t width = 4 * (dim + 2);
  size_t height = 3 * (dim + 2);
  LinearImage image(width, height, 3);
  memset((void*)image.GetData(), 0, image.GetBytesPerRow() * height);
  return image;
}

void CubemapUtils::BindFaceFromWholeImage(Cubemap &dst, Cubemap::Face face, const LinearImage &image) {
  size_t dim = dst.GetDimension() + 2; // 每张图都会有2额外像素用于无缝衔接
  size_t x = 0;
  size_t y = 0;
  switch (face) {
    case Cubemap::Face::NX:
      x = 0, y = dim;
      break;
    case Cubemap::Face::PX:
      x = 2 * dim, y = dim;
      break;
    case Cubemap::Face::NY:
      x = dim, y = 2 * dim;
      break;
    case Cubemap::Face::PY:
      x = dim, y = 0;
      break;
    case Cubemap::Face::NZ:
      x = 3 * dim, y = dim;
      break;
    case Cubemap::Face::PZ:
      x = dim, y = dim;
      break;
  }
  LinearImage sub_image;
  sub_image.Subset(image, x + 1, y + 1, dim - 2, dim - 2);
  // 此处绑定image至cubemap
  dst.SetImageForFace(face, sub_image);
}



void CubemapUtils::SetFaceFromSingleImage(Cubemap &dst, Cubemap::Face face, const LinearImage &image) {
  if (image.GetWidth() != image.GetHeight()) {
    LOG_ERROR("SetFaceFromSingleImage", "src not cube! w:{}, h:{}", image.GetWidth(), image.GetHeight());
    return;
  }

  size_t dst_dim = dst.GetDimension();
  size_t src_dim = image.GetWidth();
  // 获取采样数（对于dst的每一个像素，需要采样几个src的像素)
  size_t sample_cnt = std::max(size_t(1), src_dim / dst_dim);
  // 因为是正方形区域，所以需要平方
  sample_cnt = std::min(size_t(256), sample_cnt * sample_cnt);

  auto& face_image = dst.GetImageForFace(face);
  for (size_t y = 0; y < dst_dim; ++y) {
    float * data = face_image.GetPixel(y, 0);

    for (size_t x = 0; x < dst_dim; ++x, data += 3) {
      Cubemap::Texel tex(0);
      for (size_t i = 0; i < sample_cnt; ++i) {
        const math::Vec2 h = Hammersley(uint32_t(i), 1.0f / sample_cnt);
        size_t u = size_t((x + h.x) * src_dim / dst_dim);
        size_t v = size_t((y + h.y) * src_dim / dst_dim);
        tex += Cubemap::ReadAt(image.GetPixel(v, u));
      }
      tex /= sample_cnt;
      Cubemap::WriteAt(tex, data);
    }
  }

}


void CubemapUtils::GenerateMipmaps(std::vector<Cubemap> &levels) {
  const Cubemap& base(levels[0]);
  size_t dim = base.GetDimension();
  size_t mip_level = 0;
  while (dim > 1) {
    dim >>= 1u;
    LinearImage image;
    Cubemap dst = CubemapUtils::CreateCubemap(image, dim);
    const Cubemap& src(levels[mip_level++]);
    CubemapUtils::DownsampleCubemapLevelBoxFilter(dst, src);
    dst.MakeSeamless();
    levels.push_back(std::move(dst));
  }
}

void CubemapUtils::DownsampleCubemapLevelBoxFilter(Cubemap &dst, const Cubemap &src) {
  size_t scale = src.GetDimension() / dst.GetDimension();
  for (int i = 0; i < 6; ++i) {
    const auto& image = src.GetImageForFace((Cubemap::Face)i);
    for (size_t y = 0; y < dst.GetDimension(); ++y) {
      Cubemap::Texel * data = static_cast<Cubemap::Texel*>((void*)dst.GetImageForFace((Cubemap::Face)i).GetPixel(0, y));
      for (size_t x = 0; x < dst.GetDimension(); ++x, ++data) {
        Cubemap::WriteAt(Cubemap::FilterAt(image, x * scale, y * scale), data);
      }
    }
  }
}

void CubemapUtils::CrossToCubemap(Cubemap &dst, const LinearImage &src) {
  for (int i = 0; i < 6; ++i) {
    Cubemap::Face face = (Cubemap::Face)i;
    for (size_t iy = 0; iy < dst.GetDimension(); ++iy) {
      Cubemap::Texel *data = static_cast<Cubemap::Texel *>((void *) dst.GetImageForFace(face).GetPixel(iy, 0));
      for (size_t ix = 0; ix < dst.GetDimension(); ++ix, ++data) {
        // 找到该面在src中对应的偏移
        size_t x = ix;
        size_t y = iy;
        size_t dx = 0;
        size_t dy = 0;
        size_t dim = std::max(src.GetHeight(), src.GetWidth()) / 4;

        switch (face) {
          case Cubemap::Face::NX:
            dx = 0, dy = dim;
            break;
          case Cubemap::Face::PX:
            dx = 2 * dim, dy = dim;
            break;
          case Cubemap::Face::NY:
            dx = dim, dy = 2 * dim;
            break;
          case Cubemap::Face::PY:
            dx = dim, dy = 0;
            break;
          case Cubemap::Face::NZ:
            dx = 3 * dim, dy = dim;
            break;
          case Cubemap::Face::PZ:
            dx = dim, dy = dim;
            break;
        }

        size_t sample_cnt = std::max(size_t(1), dim / dst.GetDimension());
        sample_cnt = std::min(size_t(256), sample_cnt * sample_cnt);
        for (size_t i = 0; i < sample_cnt; i++) {
          const math::Vec2 h = Hammersley(uint32_t(i), 1.0f / sample_cnt);
          size_t u = dx + size_t((x + h.x) * dim / dst.GetDimension());
          size_t v = dy + size_t((y + h.y) * dim / dst.GetDimension());
          Cubemap::WriteAt(Cubemap::ReadAt(src.GetPixel(v, u)), data);
        }
      }
    }
  }
}

}  // namespace our_graph::image