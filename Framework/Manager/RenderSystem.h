//
// Created by Glodxy on 2021/11/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
#include <memory>
#include "SystemBase.h"
#include "Framework/Component/Renderable.h"
#include "Backend/include/DriverEnum.h"
#include "PerViewUniform.h"
namespace our_graph {

class RenderSystem : public SystemBase<SystemID::RENDER>,
 public std::enable_shared_from_this<RenderSystem> {
 public:
  void Init() override;
  void Destroy() override;

  std::string GetSystemName() const override;

  void Render();
 private:
  void PrepareRender(std::shared_ptr<Renderable> renderable);
  void Render(std::shared_ptr<Renderable> renderable);
  void OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) override;
  // todo 根据material等进行mesh分类组合

 private:
  // 提交uniform的数据
  void PreparePerView();
  void PrepareRenderable();
  void PrepareMaterial();
  // todo:
  RenderPassParams current_param_;
  PipelineState current_state_;

  std::unique_ptr<PerViewUniform> per_view_uniform_;
  // 当前的per renderable的uniform总size
  // 当大小增大时，会重新创建
  size_t current_renderable_uniform_size_ = 0;
  BufferObjectHandle per_renderable_ubh_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
