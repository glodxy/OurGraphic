//
// Created by Glodxy on 2021/11/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
#include <memory>
#include "SystemBase.h"
#include "Framework/Component/Renderable.h"
namespace our_graph {
class RenderSystem : public SystemBase<SystemID::RENDER>,
 public std::enable_shared_from_this<RenderSystem> {
 public:
  void Init() override;
  void Destroy() override;

  std::string GetSystemName() const override;

  void Render();
 private:
  void Render(std::shared_ptr<Renderable> renderable);
  void OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) override;
  // todo 根据material等进行mesh分类组合

 private:
  PipelineState current_state_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
