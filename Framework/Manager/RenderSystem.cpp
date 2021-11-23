//
// Created by Glodxy on 2021/11/23.
//

#include "RenderSystem.h"
#include "Utils/Event/APICaller.h"
#include "MeshReader.h"
namespace our_graph {
using utils::APICaller;
void RenderSystem::Init() {
  APICaller<RenderSystem>::RegisterAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID, weak_from_this());
}

void RenderSystem::Destroy() {
  APICaller<RenderSystem>::RemoveAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID);
}

std::string RenderSystem::GetSystemName() const {
  return "RenderSystem";
}

void RenderSystem::OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) {
  // todo：处理renderable
}

void RenderSystem::Render() {
  // todo:此处目前仅处理renderable
  // todo:目前所有的都走同一个pass以及pipeline state
  for (auto& entity : components_) {
    auto& com_list = entity.second;
    auto com = GetComponentFromList(com_list, ComponentType::RENDERABLE);
    auto renderable = ComCast<Renderable>(com);
    Render(renderable);
  }
}


void RenderSystem::Render(std::shared_ptr<Renderable> renderable) {
  auto mesh_info = renderable->GetMeshInfo();
  MeshReader reader(driver_);
  reader.LoadMeshFromFile(renderable->GetMeshInfo().mesh_name);
  uint32_t size = reader.GetMeshSize();
  for (int i = 0; i < size; ++i) {
    driver_->Draw(current_state_, reader.GetPrimitiveAt(i));
  }
}

}