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
  Vec2 p1 = SoftTransform::Extract<2>(src.a->position);
  Vec2 p2 = SoftTransform::Extract<2>(src.b->position);
  Vec2 p3 = SoftTransform::Extract<2>(src.c->position);

  Rect2D<int> bbox = SoftTransform::GetTriangleBBox(p1, p2, p3);
  // 遍历包围盒
  for (int i = bbox.b; i <= bbox.t; ++i) {
    if (i < 0 || i >= height) continue;
    for (int j = bbox.l; j <= bbox.r; ++j) {
      if (j < 0 || j >= width) continue;
      // 以(.5,.5)为采样中心
      Vec3 barycentric = SoftTransform::Barycentric({j + 0.5, i + 0.5}, p1, p2, p3);
      if (barycentric.x < 0 || barycentric.y < 0 || barycentric.z < 0) {
        continue;
      }
      Pixel pixel(j, i);
      // 重心坐标插值深度
      pixel.depth =
          src.a->position.z * barycentric.x +
          src.b->position.z * barycentric.y +
          src.c->position.z * barycentric.z;
      pixel.depth = (pixel.depth + 1) / 2.f;
      SamplePixelData(barycentric, src, (CustomData*)pixel.data);
      pixels.push_back(pixel);
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
  pixel_size = pixels.size();
  for (int i = 0; i < pixel_size; ++i) {
    pixel[i] = pixels[i];
  }
}

void SimpleSoftRasterizer::SamplePixelData(
    Vec3 barycentric,
    const Triangle &src,
    CustomData *data) {
  CustomData* data_a = (CustomData*)src.a->data;
  CustomData* data_b = (CustomData*)src.b->data;
  CustomData* data_c = (CustomData*)src.c->data;
  float power_a = barycentric.x;
  float power_b = barycentric.y;
  float power_c = barycentric.z;

  data->world_position =
      data_a->world_position * power_a +
      data_b->world_position * power_b +
      data_c->world_position * power_c;
  data->world_normal =
      data_a->world_normal * power_a +
      data_b->world_normal * power_b +
      data_c->world_normal * power_c;
}

}