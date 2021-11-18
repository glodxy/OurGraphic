//
// Created by Glodxy on 2021/11/18.
//

#include "Camera.h"
#include "Transform.h"
#include "Utils/Event/APICaller.h"

namespace our_graph {
using utils::APICaller;

Camera::Camera(uint32_t id) : ComponentBase(id) {
}

Camera::~Camera() noexcept {
  APICaller<Camera>::RemoveAPIHandler(CALLER_TYPE, entity_id_);
}

void Camera::Init() {
  APICaller<Camera>::RegisterAPIHandler(CALLER_TYPE, entity_id_, weak_from_this());
}

void Camera::UpdateViewMatrix() {
  math::Vec3 pos = APICaller<Transform>::CallAPI(CALLER_TYPE, entity_id_,
                                                 &Transform::GetPosition);
  LOG_INFO("Camera", "Pos:{}-{}-{}", pos.x, pos.y, pos.z);
}

}  // namespace our_graph