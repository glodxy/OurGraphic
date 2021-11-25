//
// Created by Glodxy on 2021/11/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
#include <memory>
#include <Resource/ShaderCache.h>
#include "SystemBase.h"
#include "Framework/Component/Renderable.h"
#include "Backend/include/DriverEnum.h"
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

  // todo:
  RenderPassParams current_param_;
  PipelineState current_state_;
  ShaderCache shader_cache_;
  BufferObjectHandle per_frame_uniform_;
  BufferObjectHandle per_obj_uniform_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_RENDERSYSTEM_H_
