//
// Created by chaytian on 2021/11/12.
//

#include "SimpleSoftRasterizer.h"
#include "SoftContext.h"
#include "Utils/SoftTransform.h"
namespace our_graph {
/**
 * 单个三角形的光栅化
 * todo：视口变换以及采样
 * */
void SimpleSoftRasterizer::RasterizerSingleTriangle(const Triangle &src, std::vector<Pixel> &pixels) {
  uint32_t width = SoftContext::Get().window_width_;
  uint32_t height = SoftContext::Get().window_height_;
  Vec2 p1 = SoftTransform::Extract<2>(*src.a);
  Vec2 p2 = SoftTransform::Extract<2>(*src.b);
  Vec2 p3 = SoftTransform::Extract<2>(*src.c);

  Rect2D<int> bbox = SoftTransform::GetTriangleBBox(p1, p2, p3);
  // 遍历包围盒
  for (int i = bbox.b; i <= bbox.t; ++i) {
    if (i < 0 || i >= height) continue;
    for (int j = bbox.l; j < bbox.r; ++j) {
      if (j < 0 || j >= width) continue;
      Vec3 barycentric = SoftTransform::Barycentric({j, i}, p1, p2, p3);
      if (barycentric.x < 0 || barycentric.y < 0 || barycentric.z < 0 ||
          barycentric.x > 1.f || barycentric.y > 1.f || barycentric.z > 1.f) {
        continue;
      }
      pixels.push_back(Pixel(j, i));
    }
  }
}

void SimpleSoftRasterizer::Rasterize(const Triangle *triangles, size_t size, Pixel *&pixel, size_t &pixel_size) {
  std::vector<Pixel> pixels;
  // 遍历所有三角形
  for (int i = 0; i < size; ++i) {
    RasterizerSingleTriangle(triangles[i], pixels);
  }
  pixel = new Pixel[pixels.size()];
  memcpy(pixel, pixels.data(), sizeof(Pixel) * pixels.size());
  pixel_size = pixels.size();
}

}