//
// Created by Glodxy on 2021/11/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_UTILS_SOFTTRANSFORM_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_UTILS_SOFTTRANSFORM_H_
#include "../Base/SoftDataDef.h"
namespace our_graph {
/**
 * !!!!GLM的矩阵按列存储，即[0]访问第一列
 * */
class SoftTransform {
 public:
  /**
   * 获取平移矩阵
   * @param position：平移的坐标
   * */
  static Mat4 Translation(Vec3 position);

  /**
   * 获取绕轴旋转的矩阵
   * @param axis：旋转轴
   * @param euler_angle:旋转角度
   * */
  static Mat4 Rotate(Vec3 axis, float euler_angle);

  static Mat4 RotateX(float euler_angle);
  static Mat4 RotateY(float euler_angle);
  static Mat4 RotateZ(float euler_angle);

  /**
   * 获取缩放矩阵
   * @param ratio:三个轴的缩放比率
   * */
  static Mat4 Scale(Vec3 ratio);

  /**
   * 获取观察矩阵
   * 用于将物体转换到相机坐标系
   * 相机位于原点，看向-z。使用右手系
   * @param position: 相机位置
   * @param lookat: 看向的方向
   * @param up: 定义的上方向
   * */
  static Mat4 View(Vec3 position, Vec3 lookat, Vec3 up);

  /**
   * 正交投影
   * @param bound：正交投影定义的视锥体边界
   * */
  static Mat4 Ortho(Rect3D bound);

  /**
   * 透视投影
   * @param frustum：视锥体的参数
   * */
  static Mat4 Perspective(Frustum frustum);
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_UTILS_SOFTTRANSFORM_H_
