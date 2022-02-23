//
// Created by Glodxy on 2021/11/18.
//

#include "Transform.h"
#include "Utils/Event/APICaller.h"
#include "Utils/Math/TransformUtils.h"
namespace our_graph {
using utils::APICaller;
Transform::Transform(uint32_t id) : ComponentBase(id){
}

Transform::~Transform() noexcept {
  APICaller<Transform>::RemoveAPIHandler(CALL_COMPONENT, entity_id_);
}

void Transform::Init() {
  APICaller<Transform>::RegisterAPIHandler(CALL_COMPONENT, entity_id_, weak_from_this());
}

uint32_t Transform::GetComponentType() const {
  return TRANSFORM;
}

math::Vec3 Transform::GetPosition() {
  return position_;
}

math::Mat4 Transform::GetModelMatrix() {
  return  math::TransformUtils::Translation(position_)*
          math::TransformUtils::RotateZ(rotate_.z)*
          math::TransformUtils::RotateY(rotate_.y)*
          math::TransformUtils::RotateX(rotate_.x)*
          math::TransformUtils::Scale(scale_);
}

void Transform::SetPosition(math::Vec3 pos) {
  position_ = pos;
}

void Transform::SetRotate(math::Vec3 rotate) {
  rotate_ = rotate;
}

void Transform::SetScale(math::Vec3 scale) {
  scale_ = scale;
}

}