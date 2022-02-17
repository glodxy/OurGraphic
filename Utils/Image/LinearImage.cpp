//
// Created by Glodxy on 2022/2/10.
//

#include "LinearImage.h"

namespace our_graph::image {
LinearImage::SharedReference::SharedReference(uint32_t width, uint32_t height, uint32_t channel) {
  const uint32_t nfloats = width * height * channel;
  float* floats = new float[nfloats];
  memset(floats, 0, sizeof(float) * nfloats);
  pixels = std::shared_ptr<float>(floats, std::default_delete<float[]>());
}

LinearImage::LinearImage(const LinearImage &rhs) {
  *this = rhs;
}

LinearImage & LinearImage::operator=(const LinearImage &rhs) {
  auto new_data = rhs.data_ ? new SharedReference(*rhs.data_ref_) : nullptr;
  // 此处销毁数据的引用，如果没有地方持有了，则此处会直接销毁数据
  delete data_ref_;
  data_ref_ = new_data;
  data_ = rhs.data_;
  w_ = rhs.w_;
  h_ = rhs.h_;
  channel_ = rhs.channel_;
  ppr_ = rhs.ppr_;

  return *this;
}

LinearImage::LinearImage(uint32_t w, uint32_t h, uint32_t channel) :
  data_ref_(new SharedReference(w, h, channel)), w_(w), h_(h), channel_(channel), data_(data_ref_->pixels.get()) {
  ppr_ = w;
}


void LinearImage::Subset(const LinearImage &image, size_t x, size_t y, size_t w, size_t h) {
  auto new_data = image.data_ ? new SharedReference(*image.data_ref_) : nullptr;
  // 此处销毁数据的引用，如果没有地方持有了，则此处会直接销毁数据
  delete data_ref_;
  data_ref_ = new_data;
  data_ = image.data_ + (x + y * image.w_) * image.channel_;
  w_ = w;
  h_ = h;
  channel_ = image.channel_;

  //! 对于sub image，需要使用原image的bpr
  ppr_ = image.ppr_;
}


LinearImage::~LinearImage() {
  delete data_ref_;
}

}