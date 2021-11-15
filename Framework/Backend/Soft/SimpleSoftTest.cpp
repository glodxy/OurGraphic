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
  memcpy(dst_pixel, pixels.data(), sizeof(Pixel) * dst_size);
}

void SimpleSoftTest::DepthTest(std::vector<Pixel> &pixels) {
  std::vector<Pixel> src;
  pixels.swap(src);
  SoftRenderTarget* rt = (SoftRenderTarget*) context_;
  for (const auto& p : src) {
    if (p.depth < rt->GetDepth(p.x, p.y)) {
      rt->SetDepth(p.x, p.y, p.depth);
      pixels.push_back(p);
    }
  }
}
}