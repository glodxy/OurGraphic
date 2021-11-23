//
// Created by Glodxy on 2021/11/22.
//

#include "Renderable.h"
#include "Utils/Event/APICaller.h"
namespace our_graph {
using utils::APICaller;

Renderable::Renderable(uint32_t id) : ComponentBase(id) {}

void Renderable::Init() {
  APICaller<Renderable>::RegisterAPIHandler(CALL_COMPONENT, entity_id_, weak_from_this());
}

Renderable::~Renderable() noexcept {
  APICaller<Renderable>::RemoveAPIHandler(CALL_COMPONENT, entity_id_);
}

SystemID Renderable::GetSystemID() const {
  return SystemID::RENDER;
}

uint32_t Renderable::GetComponentType() const {
  return RENDERABLE;
}

Renderable::MeshInfo Renderable::GetMeshInfo() const {
  return mesh_info_;
}

}