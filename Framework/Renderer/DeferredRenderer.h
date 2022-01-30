//
// Created by 77205 on 2022/1/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderGraphResourceAllocator.h"
#include "SceneRenderer.h"
namespace our_graph {
class DeferredRenderer : SceneRenderer {
 public:
  explicit DeferredRenderer(const SceneViewFamily* input, Driver* driver);
 public:
  void Render(render_graph::RenderGraph &graph) override;

 private:
  void InitGBuffer();
 private:
  //! 几何pass
  void PrepareGeometryPass();
  //! 光照pass
  void PrepareLightPass();

  //! 清除gbuffer
  void ClearGBuffer();
 private:
  // gbuffer
  struct RenderTargetData {
    RenderTargetHandle handle;
    render_graph::RenderGraphRenderPassInfo::ExternalDescriptor desc;
  };
  RenderTargetData gbuffer_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
