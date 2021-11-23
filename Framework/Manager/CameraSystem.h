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

  // 获取相机对应的render target
  RenderTargetHandle GetRenderTarget(const std::string& key);

  std::shared_ptr<Camera> GetMainCamera();
 protected:
  void OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) override;

 private:
  std::shared_ptr<Camera> main_camera_ {nullptr};

  // render target的缓存
  std::map<std::string, RenderTargetHandle> render_target_cache_;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_CAMERASYSTEM_H_
