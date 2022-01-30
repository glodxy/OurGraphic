//
// Created by 77205 on 2022/1/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
#include "IRenderer.h"
#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderGraphResourceAllocator.h"
namespace our_graph {
class DeferredRenderer : IRenderer {
 public:
  explicit DeferredRenderer(Driver* driver);
 public:
  void Execute(const PerViewData &per_view, const std::vector<PerRenderableData> &renderables) override;

 private:
  //! 几何pass
  void PrepareGeometryPass();
  void PrepareLightPass();
 private:
  render_graph::RenderGraph* render_graph_;
  render_graph::RenderGraphResourceAllocator allocator_;

 private:
  PerViewData current_per_view_;
  std::vector<PerRenderableData> current_per_renderable_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
