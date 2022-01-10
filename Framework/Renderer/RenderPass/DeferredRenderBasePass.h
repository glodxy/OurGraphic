//
// Created by Glodxy on 2022/1/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERPASS_DEFERREDRENDERBASEPASS_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERPASS_DEFERREDRENDERBASEPASS_H_
#include <vector>

#include "IRenderPass.h"
#include "Backend/include/Handle.h"
#include "Component/Camera.h"
#include "Component/Renderable.h"
#include "Resource/include/RenderTarget.h"
namespace our_graph {

namespace renderer {
/**
 * 该类描述了deferred render中的base pass
 * 即用于写入gbuffer的pass
 * input:gbuffer render_target
 *      visible renderable
 *      camera
 *
 * setup:1. 设置gbuffer
 *      2. 更新PerView数据
 *      3. 更新PerRenderable
 *      4. 更新PerMaterial
 *
 * execute:1.绑定RenderTarget
 *         2. 渲染Renderable，绑定相应的Material以及Primitive
 * */
class DeferredRenderBasePass : public IRenderPass {
  using Renderable = our_graph::Renderable;
  using Camera = our_graph::Camera;
 public:
  explicit DeferredRenderBasePass(Driver *driver,
                                  RenderTarget * rt,
                                  std::vector<std::shared_ptr<Renderable>> renderables,
                                  std::shared_ptr<Camera> camera);
  void Setup() override;
  void Execute() override;
 protected:
  // 要渲染到的gbuffer
  RenderTarget* gbuffer_;
  std::vector<std::shared_ptr<Renderable>>  render_objs_;
  std::shared_ptr<Camera> camera_;
};

}  // namespace renderer
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERPASS_DEFERREDRENDERBASEPASS_H_
