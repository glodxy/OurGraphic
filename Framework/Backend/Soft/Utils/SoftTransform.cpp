//
// Created by Glodxy on 2021/11/5.
//

#include "SoftTransform.h"

namespace our_graph {

Mat4 SoftTransform::Translation(Vec3 position) {
  Mat4 res = Mat4 (1);
  res[3][0] = position.x;
  res[3][1] = position.y;
  res[3][2] = position.z;
  return res;
}

Mat4 SoftTransform::RotateX(float euler_angle) {
  Mat4 res = Mat4(1);
  res[1] = {0, glm::cos(euler_angle), -glm::sin(euler_angle), 0};
  res[2] = {0, glm::sin(euler_angle), glm::cos(euler_angle), 0};
  return res;
}

Mat4 SoftTransform::RotateY(float euler_angle) {
  Mat4 res = Mat4(1);
  res[0] = {glm::cos(euler_angle), 0, glm::sin(euler_angle), 0};
  res[2] = {-glm::sin(euler_angle), 0, glm::cos(euler_angle), 0};
  return res;
}

Mat4 SoftTransform::RotateZ(float euler_angle) {
  Mat4 res = Mat4(1);
  res[0] = {glm::cos(euler_angle), -glm::sin(euler_angle), 0, 0};
  res[1] = {glm::sin(euler_angle), glm::cos(euler_angle), 0, 0};
  return res;
}

/**
 * 使用罗德里格斯旋转公式来构造矩阵
 * */
Mat4 SoftTransform::Rotate(Vec3 axis, float euler_angle) {
  float cos_v = glm::cos(euler_angle);
  float sin_v = glm::sin(euler_angle);

  Mat3 cross_v = glm::outerProduct(axis, axis);
  Mat3 n = {
      {0, -axis.z, -axis.y},
      {axis.z, 0, -axis.x},
      {-axis.y, axis.x, 0}
  };
  Mat3 res_rect = cos_v * Mat3(1) + (1 - cos_v) * cross_v + sin_v * n;
  Mat4 res = Mat4(1);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      res[i][j] = res_rect[i][j];
    }
  }
  return res;
}

Mat4 SoftTransform::Scale(Vec3 ratio) {
  Mat4 res = Mat4(1);
  res[0][0] = ratio.x;
  res[1][1] = ratio.y;
  res[2][2] = ratio.z;
  return res;
}

Mat4 SoftTransform::View(Vec3 position, Vec3 lookat, Vec3 up) {
  // 获取归一化之后的向量
  Vec3 i_lookat = glm::normalize(lookat);
  Vec3 i_up = glm::normalize(up);

  // 正交化
  Vec3 i_right = glm::cross(i_lookat, i_up);
  i_up = glm::cross(i_right, i_lookat);

  // 此处得到了正交化的三个方向的单位向量
  // 以i_right为x， i_up为y, i_lookat为-z
  Mat4 rotate = Mat4(1);
  rotate[0] = {i_right.x, i_up.x, -i_lookat.x, 0};
  rotate[1] = {i_right.y, i_up.y, -i_lookat.y, 0};
  rotate[2] = {i_right.z, i_up.z, -i_lookat.z, 0};

  Mat4 translation = Translation(-position);
  return translation * rotate;
}

Mat4 SoftTransform::Ortho(Rect3D bound) {
  Mat4 translation = Translation(
      {-(bound.r+bound.l)/2,
       -(bound.t+bound.b)/2,
       -(bound.n+bound.f)/2});
  Mat4 scale = Scale(
      {2/(bound.r-bound.l),
       2/(bound.t-bound.b),
       2/(bound.n-bound.f)});
  Mat4 res = scale * translation;
  return res;
}

Mat4 SoftTransform::Perspective(Frustum frustum) {
  Rect3D rect;
  float angle = frustum.fov/2;
  rect.t = glm::abs(glm::tan(angle) * frustum.n);
  rect.b = -rect.t;
  rect.r = rect.t * frustum.aspect;
  rect.l = -rect.r;
  rect.n = frustum.n;
  rect.f = frustum.f;
  Mat4 transfer = {
      {frustum.n, 0, 0, 0},
      {0, frustum.n, 0, 0},
      {0, 0, frustum.n + frustum.f, 1},
      {0, 0, -frustum.n * frustum.f, 0}
  };
  Mat4 ortho = Ortho(rect);
  Mat4 res = ortho * glm::transpose(transfer);
  return res;
}

Vec3 SoftTransform::Barycentric(Vec2 p, Vec2 p1, Vec2 p2, Vec2 p3) {
  Vec3 t_p(p, 0);
  Vec3 t_p1(p1, 0);
  Vec3 t_p2(p2, 0);
  Vec3 t_p3(p3, 0);
  float S1 = glm::length(glm::cross((t_p3 - t_p2), (t_p - t_p2)));
  float S2 = glm::length(glm::cross((t_p1 - t_p3), (t_p - t_p3)));
  float S3 = glm::length(glm::cross((t_p2 - t_p1), (t_p - t_p1)));
  float S = glm::length(glm::cross((t_p2 - t_p1), (t_p3 - t_p1)));
  Vec3 res;
  res.x = (S1 / S);
  res.y = (S2 / S);
  res.z = (S3 / S);
  return res;
}

Rect2D<int> SoftTransform::GetTriangleBBox(Vec2 p1, Vec2 p2, Vec2 p3) {
  Rect2D<int> rect;
  float l = glm::min(glm::min(p1.x, p2.x), p3.x);
  float b = glm::min(glm::min(p1.y, p2.y), p3.y);
  float r = glm::max(glm::max(p1.x, p2.x), p3.x);
  float t = glm::max(glm::max(p1.y, p2.y), p3.y);

  rect.b = glm::floor(b);
  rect.l = glm::floor(l);
  rect.r = glm::floor(r);
  rect.t = glm::floor(t);
  return rect;
}
}