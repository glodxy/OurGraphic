//
// Created by Glodxy on 2021/11/18.
//

#include "Transform.h"
#include "Utils/Event/APICaller.h"
namespace our_graph {
using utils::APICaller;
Transform::Transform(uint32_t id) : ComponentBase(id){
}

void Transform::Init() {
  APICaller<Transform>::RegisterAPIHandler(CALLER_TYPE, entity_id_, weak_from_this());
}

math::Vec3 Transform::GetPosition() {
  return position_;
}

void Transform::SetPosition(math::Vec3 pos) {
  position_ = pos;
}
}