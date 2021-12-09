//
// Created by Glodxy on 2021/11/18.
//

#ifndef OUR_GRAPHIC_UTILS_MATH_H_
#define OUR_GRAPHIC_UTILS_MATH_H_
#include "glm/glm.hpp"

namespace our_graph::math {
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

}
#endif //OUR_GRAPHIC_UTILS_MATH_H_
