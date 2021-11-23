//
// Created by Glodxy on 2021/11/17.
//

#include "SystemManager.h"
#include "RenderSystem.h"
#include "CameraSystem.h"
namespace our_graph {
SystemManager::SystemManager() {
  system_map_.insert_or_assign(SystemID::CAMERA, std::make_shared<CameraSystem>());
  system_map_.insert_or_assign(SystemID::RENDER, std::make_shared<RenderSystem>());
}

void SystemManager::Init() {
  for (auto& iter : system_map_) {
    iter.second->Init();
  }
}

void SystemManager::Close() {
  for (auto& iter : system_map_) {
    iter.second->Destroy();
  }
}

ISystem * SystemManager::GetSystem(SystemID id) {
  if (system_map_.find(id) != system_map_.end()) {
    return system_map_[id].get();
  }
  return nullptr;
}



}