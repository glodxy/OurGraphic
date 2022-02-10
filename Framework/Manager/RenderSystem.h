//
// Created by Glodxy on 2021/11/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
#include <memory>
#include "SystemBase.h"
#include "Framework/Component/Renderable.h"
#include "Framework/Component/PerViewUniform.h"
#include "Framework/Component/SkySource.h"
#include "Framework/Renderer/IRenderer.h"
namespace our_graph {

class RenderSystem : public SystemBase<SystemID::RENDER>,
 public std::enable_shared_from_this<RenderSystem> {
 public:
  void Init() override;
  void Destroy() override;

  std::string GetSystemName() const override;

  void Update(uint32_t time) override;
  void Render();
  void OnCameraUpdate();

  std::shared_ptr<SkySource> GetSky();
 private:
  void OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) override;
  // todo 根据material等进行mesh分类组合
 private:
  std::shared_ptr<SkySource> sky_;
  std::shared_ptr<IRenderer> renderer_;
  std::vector<std::shared_ptr<Renderable>> renderables_;

  // todo：用于light pass的param
  RenderPassParams quad_param_;
  // todo:用于light pass的state
  PipelineState quad_state_;

};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
