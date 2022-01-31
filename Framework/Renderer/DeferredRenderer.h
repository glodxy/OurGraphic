//
// Created by 77205 on 2022/1/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_
#include "RenderGraph/RenderGraph.h"
#include "SceneRenderer.h"
#include "Resource/include/RenderTarget.h"
#include "Resource/include/Texture.h"
#include "include/GlobalEnum.h"
namespace our_graph {
class DeferredRenderer : SceneRenderer {
 public:
  explicit DeferredRenderer(const SceneViewFamily* input, Driver* driver);
 public:
  void Render() override;

 private:
  // 此处完成resource以及used flags的设置
  void InitGBuffer();
 private:
  //! 几何pass
  void PrepareGeometryPass(render_graph::RenderGraph& graph);
  //! 光照pass
  void PrepareLightPass(render_graph::RenderGraph& graph);

  //! 清除gbuffer
  void ClearGBuffer();
 private:
  // 默认的渲染目标
  RenderTargetHandle default_rt_;

  struct GBufferData {
    render_graph::RenderGraphId<render_graph::RenderGraphTexture> textures[GBUFFER_MAX_SIZE];
  };
  GBufferData gbuffer_data_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_DEFERREDRENDERER_H_