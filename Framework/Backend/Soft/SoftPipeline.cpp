//
// Created by chaytian on 2021/11/2.
//

#include "SoftPipeline.h"
#include "SoftContext.h"
#include "SDL2/SDL.h"

namespace our_graph {
static uint32_t GetColor(const Color& color) {
  return (uint32_t(color.r) << 24) |
      (uint32_t(color.g) << 16) |
      (uint32_t(color.b) << 8) |
      (uint32_t(color.a));
}

void SoftPipeline::Rasterize(const Triangle *triangles, size_t size, Pixel *&pixel, size_t &pixel_size) {
  uint32_t width = SoftContext::Get().window_width_;
  pixel = new Pixel[10*width];
  pixel_size = width * 10;
  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < 10; ++j) {
      pixel[i].x = i;
      pixel[i].y = 200 + j;
      pixel[i].color.r = 255;
    }
  }
}

/**
 * 目前不进行任何测试，仅拷贝像素
 * */
void SoftPipeline::Test(const Pixel *src_pixel, size_t src_size, Pixel *&dst_pixel, size_t &dst_size) {
  dst_size = src_size;
  dst_pixel = new Pixel[dst_size];
  memcpy(dst_pixel, src_pixel, sizeof(Pixel) * dst_size);
}

/**
 * 此处假设x为列，y为行
 * */
void SoftPipeline::PixelBlit(const Pixel *pixel, size_t size) {
  SDL_Texture* current_texture = SoftContext::Get().current_tex_;
  assert(current_texture);

  void* tex;
  int pitch;
  SDL_LockTexture(current_texture, nullptr, &tex, &pitch);
  for (int i = 0; i < size; ++i) {
    const Pixel p = pixel[i];
    uint32_t idx = p.y * (pitch/4) + p.x;
    ((uint32_t*)tex)[idx] = GetColor(p.color);
    if (p.color.num != 0) {
      LOG_INFO("Pixel", "{}-{}, color:{}", p.x, p.y, p.color.num);
    }
  }
  LOG_INFO("SoftPipeline", "PixelBlit {} pixels", size);
  SDL_UnlockTexture(current_texture);
}

void SoftPipeline::DestroyVertex(Vertex *&vertex, size_t size) {
  if (!vertex) {return;}
  delete [] vertex;
  vertex = nullptr;
}

void SoftPipeline::DestroyTriangle(Triangle *&triangle, size_t size) {
  if (!triangle) {return;}
  delete [] triangle;
  triangle = nullptr;
}

void SoftPipeline::DestroyPixel(Pixel *&pixel, size_t size) {
  if (!pixel) {return;}
  delete [] pixel;
  pixel = nullptr;
}

}