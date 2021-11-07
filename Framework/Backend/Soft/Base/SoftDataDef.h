//
// Created by Glodxy on 2021/11/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_SOFTDATADEF_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_SOFTDATADEF_H_
#include "glm/glm.hpp"

namespace our_graph {

typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;
typedef glm::mat3x3 Mat3;
typedef glm::mat4x4 Mat4;

struct Rect3D {
  float l, r; // 左/右
  float t, b; // 顶/底
  float n, f; // 近/远
};

template<class T = float>
struct Rect2D {
  T l, r; // 左/右
  T t, b; // 顶/底
};

struct Frustum {
  float fov;
  float aspect;
  float n,f;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_SOFTDATADEF_H_
