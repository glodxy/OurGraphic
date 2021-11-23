//
// Created by Glodxy on 2021/11/18.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_CAMERA_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_CAMERA_H_
#include <memory>
#include "ComponentBase.h"
#include "Utils/Math/Math.h"
#include "Utils/Math/TransformUtils.h"
namespace our_graph {
/**
 * 该类为camera组件类
 * 只用于存储camera相关的特有数据，包括fov，near等专用属性。
 * 实际的逻辑变换以及
 * */
class Camera : public ComponentBase,
    public std::enable_shared_from_this<Camera> {
 public:
  explicit Camera(uint32_t id);
  ~Camera() override;

  SystemID GetSystemID() const override {
    return SystemID::CAMERA;
  }
  math::Mat4 GetViewMatrix() const;
  math::Mat4 GetProjMatrix() const;

  bool IsMain() const {
    return is_main_;
  }

  // todo:获取render target的key，目前返回default
  std::string GetRenderTarget() const;

  uint32_t GetComponentType() const override;
 protected:
  void Init() override;
 private:
  math::Frustum frustum_;
  math::Vec3 lookat_;
  math::Vec3 up_;
  bool is_main_ {false};
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_CAMERA_H_
