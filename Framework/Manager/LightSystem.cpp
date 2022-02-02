//
// Created by Glodxy on 2022/2/2.
//

#include "LightSystem.h"
#include "Utils/Event/APICaller.h"
namespace our_graph {
using utils::APICaller;

void LightSystem::Init() {
  APICaller<LightSystem>::RegisterAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID, weak_from_this());
}

void LightSystem::Destroy() {
  APICaller<LightSystem>::RemoveAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID);
}

void LightSystem::OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) {
  auto light = ComCast<LightSource>(com);
  if (!light->IsStatic()) {
    dynamic_lights_.push_back(light);
  }
}

std::vector<std::shared_ptr<LightSource>> LightSystem::GetAllDynamicLight() {
  return dynamic_lights_;
}


}  // namespace our_graph