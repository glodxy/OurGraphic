//
// Created by Glodxy on 2022/2/12.
//

#include "PackUtils.h"


namespace our_graph::math {

Vec4 PackUtils::PackTangentFrame(Vec3 normal, Vec3 tangents) {
  Vec3 c = glm::cross(normal, tangents);
  Mat3 mat = Mat3(tangents, c, normal);
  glm::quat qua(mat);
  qua = glm::normalize(qua);
  qua = qua.w < 0 ? -qua : qua;

  const size_t storage_size = 2;
  const float bias = 1.0f / ((1 << (storage_size * 8 - 1)) - 1);
  if (qua.w < bias) {
    const float factor = (std::sqrt(1 - (double)bias * (double)bias));
    qua *= factor;
    qua.w = bias;
  }

  const Vec3 b = glm::cross(tangents, normal);



  if (glm::dot(glm::cross(tangents, normal), b) < 0) {
    qua = -qua;
  }

  return Vec4(qua.x, qua.y, qua.z, qua.w);
}

}