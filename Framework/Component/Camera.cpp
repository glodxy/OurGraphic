//
// Created by Glodxy on 2021/11/18.
//

#include "Camera.h"
#include "Transform.h"
#include "Utils/Event/APICaller.h"

namespace our_graph {
using utils::APICaller;

Camera::Camera(uint32_t id, bool is_main) : ComponentBase(id), is_main_(is_main) {
}

Camera::~Camera() noexcept {
  APICaller<Camera>::RemoveAPIHandler(CALL_COMPONENT, entity_id_);
}

void Camera::Init() {
  APICaller<Camera>::RegisterAPIHandler(CALL_COMPONENT, entity_id_, weak_from_this());
}

uint32_t Camera::GetComponentType() const {
  return CAMERA;
}

math::Mat4 Camera::GetViewMatrix() const {
  math::Vec3 pos = APICaller<Transform>::CallAPI(CALL_COMPONENT, entity_id_,
                                                 &Transform::GetPosition);
  return math::TransformUtils::View(pos, lookat_, up_);
}

math::Mat4 Camera::GetProjMatrix() const {
  return math::TransformUtils::Perspective(frustum_);
}

math::Rect2D<float> Camera::GetViewport() const {
  return math::TransformUtils::FrustumToViewport(frustum_);
}

void Camera::Lookat(math::Vec3 point) {
  math::Vec3 pos = APICaller<Transform>::CallAPI(CALL_COMPONENT, entity_id_,
                                                 &Transform::GetPosition);
  lookat_ = glm::normalize(point - pos);
}

std::string Camera::GetRenderTarget() const {
  return "default";
}

}  // namespace our_graph