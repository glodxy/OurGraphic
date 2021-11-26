//
// Created by Glodxy on 2021/11/26.
//

#include "include/Texture.h"

#include <array>

namespace our_graph {
struct Texture::Detail {
  Driver* driver = nullptr;
  uint32_t width = 1; // 宽
  uint32_t height = 1; // 高
  uint32_t depth = 1; // 深度
  uint8_t levels = 1; // mipmap级别
  Sampler sampler = Sampler::SAMPLER_2D;
  InternalFormat format = InternalFormat::RGBA8;
  Usage usage = Usage::DEFAULT;
  bool texture_is_swizzle = false;
  std::array<Swizzle, 4> swizzles = {
      Swizzle ::CHANNEL_0, Swizzle ::CHANNEL_1,
      Swizzle::CHANNEL_2, Swizzle::CHANNEL_3
  };
};


using BuilderType = Texture;
BuilderType::Builder::Builder(Driver* driver) noexcept {
  impl_->driver = driver;
}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

Texture::Builder & Texture::Builder::Width(uint32_t width) noexcept {
  impl_->width = width;
  return *this;
}

Texture::Builder & Texture::Builder::Height(uint32_t height) noexcept {
  impl_->height = height;
  return *this;
}

Texture::Builder & Texture::Builder::Depth(uint32_t depth) noexcept {
  impl_->depth = depth;
  return *this;
}

Texture::Builder & Texture::Builder::Levels(uint8_t levels) noexcept {
  impl_->levels = levels;
  return *this;
}

Texture::Builder & Texture::Builder::Sampler(Texture::Sampler target) noexcept {
  impl_->sampler = target;
  return *this;
}

Texture::Builder & Texture::Builder::Format(InternalFormat format) noexcept {
  impl_->format = format;
  return *this;
}

Texture::Builder & Texture::Builder::Usage(Texture::Usage usage) noexcept {
  impl_->usage = usage;
  return *this;
}

Texture::Builder & Texture::Builder::Swizzle(
    Texture::Swizzle r, Texture::Swizzle g,
    Texture::Swizzle b, Texture::Swizzle a) noexcept {
  impl_->texture_is_swizzle = true;
  impl_->swizzles = {r, g, b, a};
  return *this;
}


}