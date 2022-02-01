//
// Created by glodxy on 2022/1/30.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_SCENERENDERER_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_SCENERENDERER_H_
#include <vector>
#include "IRenderer.h"
#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderGraphResourceAllocator.h"
#include "Utils/Math/Math.h"
#include "Resource/include/UniformStruct.h"
namespace our_graph {
class PerViewUniform;
class Camera;
class Renderable;
class MaterialInstance;
/**
 * 该参数描述了一次渲染的相关信息，包括scene，viewInfo，以及所使用的rendertarget
 * SceneViewFamily
 *          ----> Scene
 *          ----> RenderTarget
 *          ----> vector<ViewInfo>
 * */
struct SceneParams {
  std::vector<std::shared_ptr<Renderable>> renderables;
  std::vector<std::shared_ptr<Camera>> cameras;
};
/**
 * 该scene理论上应该存储了该场景中所有对象的打包数据
 * */
class Scene {
 public:
  void Init(std::vector<std::shared_ptr<Renderable>> renderables);
  void CommitAllMaterialInstance(Driver* driver);
  // 获取指定位置的material instance
  MaterialInstance* GetMaterialInstance(size_t idx);
 private:
  std::vector<MaterialInstance*> material_instances_ = {};
};
/**
 * view描述了一个scene会如何被渲染
 * 一个scene会有多个view,
 * view控制的内容包括但不限于transform矩阵等
 * */
class ViewInfo {
 public:
  void Init(Driver* driver, std::shared_ptr<Camera> camera);
  void Update(uint32_t time);
  ~ViewInfo();
  PerViewUniform* GetUniforms();

 private:
  std::shared_ptr<Camera> camera_;
  float time_;
  math::Rect2D<float> viewport_;
  PerViewUniform* per_view_uniform_;
};
/**
 * 该类用于存储所有的可见mesh
 * */
class MeshCollector {
  struct SingleMesh {
    RenderPrimitiveHandle primitive;
    PerRenderableUniformBlock uniforms;
    size_t material_instance_idx; // 所拥有的material instance的idx
  };
 public:
  void Init(Driver* driver, std::vector<std::shared_ptr<Renderable>> renderables);
  // 提交所有renderable的uniform
  void CommitPerRenderableUniforms();
  // 使用第idx的renderable uniform
  void UsePerRenderableUniform(size_t idx);
  size_t GetSize();
  RenderPrimitiveHandle GetRenderPrimitiveAt(size_t idx);

 private:
  uint32_t current_renderable_uniform_size_;
  BufferObjectHandle per_renderable_ubh_;
  // 所有的mesh
  std::vector<SingleMesh> meshes_;
  Driver* driver_;
};

class SceneRenderer  : public IRenderer {
 public:
  /**
   * 在此处会初始化相关的参数，并进行裁剪/可见性检测
   * */
  explicit SceneRenderer(Driver* driver);

  virtual void Render() override = 0;

  void Update(uint32_t time) override;

  void Reset(void *params) override;
 public:
  //! 垃圾回收
  void GC();
  // 要渲染的scene
  Scene scene_;
  // 要渲染的view(根据输入的viewFamily初始化）
  std::vector<ViewInfo> views_;
  // 所有的mesh
  MeshCollector mesh_collector_;
  // viewport
  uint32_t width_, height_;
 protected:
  render_graph::RenderGraph render_graph_;
  render_graph::RenderGraphResourceAllocator allocator_;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_SCENERENDERER_H_
