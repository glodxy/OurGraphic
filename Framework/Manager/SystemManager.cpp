//
// Created by Glodxy on 2021/11/17.
//

#include "SystemManager.h"
#include "RenderSystem.h"
#include "CameraSystem.h"
namespace our_graph {
Driver* ISystem::driver_ = nullptr;

SystemManager::SystemManager() {
  system_map_.insert_or_assign(SystemID::CAMERA, std::make_shared<CameraSystem>());
  system_map_.insert_or_assign(SystemID::RENDER, std::make_shared<RenderSystem>());
}

void SystemManager::Init(Driver* driver) {
  ISystem::driver_ = driver;
  driver_ = driver;
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

void SystemManager::Update(uint32_t frame) {
  // 渲染部分
  auto render_system = SystemCast<RenderSystem>(system_map_[SystemID::RENDER]);
  // todo:timer
  uint32_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
  ).count();
  for (auto& pair : system_map_) {
    pair.second->Update(time);
  }
  // 进行渲染逻辑
  render_system->Render();
}


}