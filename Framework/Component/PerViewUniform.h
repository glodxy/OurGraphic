//
// Created by Glodxy on 2022/1/3.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_PERVIEWUNIFORM_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_PERVIEWUNIFORM_H_
#include <memory>
#include "Backend/include/Handle.h"
#include "Resource/include/TypedUniformBuffer.h"
#include "Resource/include/UniformStruct.h"
#include "Backend/include/SamplerGroup.h"
#include "Backend/include/Driver.h"
#include "Utils/Math/Math.h"
namespace our_graph {
class Camera;
class Skybox;
class PerViewUniform {
 public:
  PerViewUniform(Driver* driver);
  void PrepareLight(uint32_t dynamic_count);
  // 计算camera相关的数据
  void PrepareCamera(std::shared_ptr<Camera> camera_component);

  void PrepareViewport(math::Rect2D<float> rect);

  void PrepareTime(uint32_t time);

  // 提交资源到gpu
  void Commit();
  // 绑定资源
  void Bind();

  void Destroy();

  void PrepareSkybox(Skybox* sky);
 private:

  TypedUniformBuffer<PerViewUniformBlock> per_view_uniform_;
  SamplerGroup per_view_sampler_;
  SamplerGroupHandle per_view_sbh_;
  BufferObjectHandle per_view_ubh_;
  Driver * driver_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_PERVIEWUNIFORM_H_
