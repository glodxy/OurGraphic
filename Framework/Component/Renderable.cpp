//
// Created by Glodxy on 2021/11/22.
//

#include "Renderable.h"
#include "Framework/Resource/MaterialCache.h"
#include "Utils/Event/APICaller.h"
#include "Framework/IRenderProcessor.h"
namespace our_graph {
using utils::APICaller;

Renderable::Renderable(uint32_t id, std::string mesh, std::string material_file) : ComponentBase(id) {
  mesh_info_.mesh_name = mesh;
  // todo（important）:暂时使用processor中的driver，后续考虑增加engine层
  Material* src_mat = MaterialCache::GetMaterial(material_file, IRenderProcessor::GetDriver());
  material_instance_ = MaterialInstance::Duplicate(src_mat->GetDefaultInstance(), "");
}

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

MaterialInstance *Renderable::GetMaterialInstance() {
  return material_instance_;
}

}