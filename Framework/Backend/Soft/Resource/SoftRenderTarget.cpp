//
// Created by chaytian on 2021/11/10.
//

#include "SoftRenderTarget.h"
#include <climits>
#include "Backend/Soft/SoftContext.h"
namespace our_graph {

SoftRenderTarget::SoftRenderTarget() : IRenderTarget(), offscreen_(false) {
  uint32_t w = SoftContext::Get().window_width_;
  uint32_t h = SoftContext::Get().window_height_;
  width_ = w;
  height_ = h;
  colors_.resize(w * h, {0});
  depth_.resize(w * h, std::numeric_limits<float>::max());
}

SoftRenderTarget::SoftRenderTarget(uint32_t w, uint32_t h) : IRenderTarget(w, h), offscreen_(true) {
  colors_.resize(width_ * height_, {0});
}

uint32_t SoftRenderTarget::GetIndex(uint32_t x, uint32_t y) {
  return y * width_ + x;
}

uint64_t SoftRenderTarget::GetByteSize() const {
  return width_ * height_ * sizeof(Color);
}

uint32_t SoftRenderTarget::GetSize() const {
  return colors_.size();
}

const void * SoftRenderTarget::GetData() const {
  return colors_.data();
}

void SoftRenderTarget::SetPixel(int x, int y, Color color) {
  if (x < 0 || x >= width_ || y < 0 || y>= height_) {
    return;
  }
  colors_[GetIndex(x, y)] = color;
}

float SoftRenderTarget::GetDepth(int x, int y) {
  if (x < 0 || x >= width_ || y < 0 || y>= height_) {
    return 0;
  }
  return depth_[GetIndex(x, y)];
}

void SoftRenderTarget::SetDepth(int x, int y, float depth) {
  if (x < 0 || x >= width_ || y < 0 || y>= height_) {
    return;
  }
  depth_[GetIndex(x, y)] = depth;
}

void SoftRenderTarget::ClearColor() {
  colors_.clear();
  colors_.resize(width_ * height_, {0});
}

void SoftRenderTarget::ClearDepth() {
  depth_.resize(width_ * height_, std::numeric_limits<float>::max());
}
}  // namespace our_graph