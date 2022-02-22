//
// Created by Glodxy on 2021/11/18.
//

#ifndef OUR_GRAPHIC_UTILS_MATH_H_
#define OUR_GRAPHIC_UTILS_MATH_H_
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace our_graph::math {
constexpr const double PI  = 3.14159265358979323846264338327950288;
// 1/pi
constexpr const double I_PI     = 0.318309886183790671537767526745028724;
// 2/pi
constexpr const double II_PI     = 0.636619772367581343075535053490057448;

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Vec2i = glm::ivec2;
using Vec3i = glm::ivec3;
using Vec4i = glm::ivec4;
using Vec2u = glm::uvec2;
using Vec3u = glm::uvec3;
using Vec4u = glm::uvec4;
using Vec2b = glm::vec<2, uint32_t>;
using Vec3b = glm::vec<3, uint32_t>;
using Vec4b = glm::vec<4, uint32_t>;
using Vec3ui = glm::vec<3, uint32_t>;


static_assert(sizeof(Vec3) == (sizeof(float) * 3));

using Mat2 = glm::mat2;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

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


template<typename T>
static inline constexpr T Sq(T x) {
  return x * x;
}

template<typename T>
static inline constexpr T Log4(T x) {
  // log2(x)/log2(4)
  // log2(x)/2
  return std::log2(x) * T(0.5);
}

}
#endif //OUR_GRAPHIC_UTILS_MATH_H_
