//
// Created by Glodxy on 2021/11/21.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_CAMERASYSTEM_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_CAMERASYSTEM_H_
#include "SystemBase.h"
#include <memory>
#include "Component/Camera.h"
namespace our_graph {
class CameraSystem : public SystemBase<SystemID::CAMERA>,
 public std::enable_shared_from_this<CameraSystem> {
 public:
  void Init() override;
  void Destroy() override;

  std::string GetSystemName() const override;

  std::shared_ptr<Camera> GetMainCamera();
 protected:
  void OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) override;

 private:
  std::shared_ptr<Camera> main_camera_ {nullptr};
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_CAMERASYSTEM_H_
