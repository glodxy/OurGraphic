//
// Created by Home on 2022/1/30.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_SCENERENDERER_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_SCENERENDERER_H_
#include <vector>
#include "IRenderer.h"
#include "RenderGraph/RenderGraphResourceAllocator.h"
#include "Utils/Math/Math.h"
namespace our_graph {
/**
 * 该参数描述了一次渲染的相关信息，包括scene，viewInfo，以及所使用的rendertarget
 * SceneViewFamily
 *          ----> Scene
 *          ----> RenderTarget
 *          ----> vector<ViewInfo>
 * */
class SceneViewFamily;
/**
 * 该scene理论上应该存储了该场景中所有对象的打包数据
 * */
class Scene;
/**
 * view描述了一个scene会如何被渲染
 * 一个scene会有多个view,
 * view控制的内容包括但不限于transform矩阵等
 * */
class ViewInfo {};
/**
 * 该类用于存储所有的可见mesh
 * */
class MeshCollector {};
class SceneRenderer  : public IRenderer {
 public:
  /**
   * 在此处会初始化相关的参数，并进行裁剪/可见性检测
   * */
  SceneRenderer(const SceneViewFamily* input, Driver* driver);

  virtual void Render(render_graph::RenderGraph &graph) override = 0;

  //! 垃圾回收
  void GC();
 public:
  // 要渲染的scene
  Scene* scene_;
  // 要渲染的view(根据输入的viewFamily初始化）
  std::vector<ViewInfo> views_;
  // 所有的mesh
  MeshCollector mesh_collector_;
  // viewport
  uint32_t width_, height_;
 protected:
  render_graph::RenderGraphResourceAllocator allocator_;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_SCENERENDERER_H_
