//
// Created by Glodxy on 2022/2/2.
//

#include "LightSource.h"
#include "Transform.h"
#include "Utils/Event/APICaller.h"
namespace our_graph {
using utils::APICaller;
LightSource::LightSource(uint32_t id) : ComponentBase(id) {}

LightSource::~LightSource() noexcept {
  APICaller<LightSource>::RemoveAPIHandler(CALL_COMPONENT, GetEntity());
}

void LightSource::Init() {
  APICaller<LightSource>::RegisterAPIHandler(CALL_COMPONENT, GetEntity(), weak_from_this());
}

uint32_t LightSource::GetComponentType() const {
  return ComponentType::LIGHT;
}

SystemID LightSource::GetSystemID() const {
  return SystemID::LIGHT;
}

bool LightSource::IsStatic() const {
  return is_static_;
}

math::Mat4 LightSource::GetLightMat() const {
  math::Vec3 pos = APICaller<Transform>::CallAPI(CALL_COMPONENT, GetEntity(), &Transform::GetPosition);
  math::Mat4 mat(0);
  mat[0] = math::Vec4(pos, falloff_);
  mat[1] = color_;
  mat[2] = math::Vec4(direction_, intensity_);

  return mat;
}

void LightSource::SetColor(math::Vec4 color) {
  color_ = color;
}

void LightSource::SetIntensity(float intensity) {
  intensity_ = intensity;
}

}  // namespace our_graph
