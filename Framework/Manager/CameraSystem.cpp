//
// Created by Glodxy on 2021/11/21.
//

#include "CameraSystem.h"
#include "Utils/Event/APICaller.h"
#include "Component/Camera.h"
namespace our_graph {
using utils::APICaller;
void CameraSystem::Init() {
  APICaller<CameraSystem>::RegisterAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID, weak_from_this());
}

void CameraSystem::Destroy() {
  APICaller<CameraSystem>::RemoveAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID);
}

std::string CameraSystem::GetSystemName() const {
  return "CameraSystem";
}

std::shared_ptr<Camera> CameraSystem::GetMainCamera() {
  return main_camera_;
}

void CameraSystem::OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) {
  auto camera = ComCast<Camera>(com);
  if (camera->IsMain()) {
    main_camera_ = camera;
  }
}

}