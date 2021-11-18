//
// Created by Glodxy on 2021/11/18.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_CAMERA_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_CAMERA_H_
#include <memory>
#include "ComponentBase.h"
#include "Utils/Math.h"
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

  SystemID GetSystemID() override {
    return SystemID::CAMERA;
  }
//  math::Mat4 GetViewMatrix() const;
//  math::Mat4 GetProjMatrix() const;
  // 更新view矩阵
  void UpdateViewMatrix();
  void Init() override;
 private:

  // 观察矩阵
  math::Mat4 view_;
  // 投影矩阵
  math::Mat4 proj_;
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_CAMERA_H_
