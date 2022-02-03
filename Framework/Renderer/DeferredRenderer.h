//
// Created by glodxy on 2022/1/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
#include "SceneRenderer.h"
#include "include/GlobalEnum.h"

namespace our_graph {
class DeferredRenderer : public SceneRenderer {
 public:
  explicit DeferredRenderer(Driver* driver);
  ~DeferredRenderer() = default;
 public:
  void Render() override;
  void Init() override;

  void Destroy() override;
 private:
 private:
  //! 几何pass
  void PrepareGeometryPass(render_graph::RenderGraph& graph);
  //! 光照pass
  void PrepareLightPass(render_graph::RenderGraph& graph);

 private:
  // 默认的渲染目标
  RenderTargetHandle default_rt_;
  SamplerGroupHandle gbuffer_sampler_;
  struct GBufferData {
    render_graph::RenderGraphId<render_graph::RenderGraphTexture> textures[GBUFFER_MAX_SIZE];
    render_graph::RenderGraphId<render_graph::RenderGraphTexture> ds;
  };
  GBufferData gbuffer_data_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
