//
// Created by chaytian on 2021/11/15.
//

#include "SimpleSoftTest.h"
#include "Resource/SoftRenderTarget.h"
namespace our_graph {
void SimpleSoftTest::Test(const Pixel *src_pixel, size_t src_size, Pixel *&dst_pixel, size_t &dst_size) {
  std::vector<Pixel> pixels(src_pixel, src_pixel + src_size);
  // 1.深度测试
  DepthTest(pixels);


  dst_size = pixels.size();
  dst_pixel = new Pixel[dst_size];
  for (int i = 0; i < dst_size; ++i) {
    dst_pixel[i] = pixels[i];
  }
}

void SimpleSoftTest::DepthTest(std::vector<Pixel> &pixels) {
  std::vector<Pixel> src;
  pixels.swap(src);
  SoftRenderTarget* rt = (SoftRenderTarget*) context_;
  size_t size = rt->GetSize();
  std::vector<const Pixel*> tmp_pixels;
  tmp_pixels.resize(size, nullptr);
  for (const auto& p : src) {
    if (p.depth < rt->GetDepth(p.x, p.y)) {
      rt->SetDepth(p.x, p.y, p.depth);
      tmp_pixels[rt->GetIndex(p.x, p.y)] = &p;
    }
  }
  // 写回
  for (auto iter : tmp_pixels) {
    if (iter) {
      pixels.push_back(*iter);
    }
  }
}
}