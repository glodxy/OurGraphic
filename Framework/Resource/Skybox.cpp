//
// Created by Glodxy on 2022/2/9.
//

#include "Resource/include/Skybox.h"

#include "Resource/include/Texture.h"
#include "Resource/ResourceAllocator.h"
namespace our_graph {

struct Skybox::Detail {
  Texture* env = nullptr;
  math::Vec4 color {0, 0, 0, 1};
  // todo:间接光照默认值
  float intensity = 30000.0f;
  bool has_sun = false;
  Driver* driver = nullptr;
};

using BuilderType = Skybox;
BuilderType::Builder::Builder(Driver* driver) noexcept {
  impl_->driver = driver;
}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;


Skybox::Builder & Skybox::Builder::Environment(Texture *tex) {
  impl_->env = tex;
  return *this;
}

Skybox::Builder & Skybox::Builder::Intensity(float intensity) {
  impl_->intensity = intensity;
  return *this;
}

Skybox::Builder & Skybox::Builder::HasSun(bool f) {
  impl_->has_sun = f;
  return *this;
}

Skybox::Builder & Skybox::Builder::Color(math::Vec4 color) {
  impl_->color = color;
  return *this;
}

Skybox * Skybox::Builder::Build() {
  if (!impl_->env->IsCubemap()) {
    LOG_ERROR("Skybox", "Build Failed! Env Not CubeMap");
    return nullptr;
  }
  return ResourceAllocator::Get().CreateResource<Skybox>(*this);
}


Skybox::Skybox(const Builder &builder)
  : env_(builder->env),
  intensity_(builder->intensity) {

}

void Skybox::Destroy() {}

float Skybox::GetIntensity() const {
  return intensity_;
}

const Texture *Skybox::GetTexture() const {
  return env_;
}

}  // namespace our_graph