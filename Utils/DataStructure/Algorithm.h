//
// Created by Glodxy on 2021/12/18.
//

#ifndef OUR_GRAPHIC_UTILS_DATASTRUCTURE_ALGORITHM_H_
#define OUR_GRAPHIC_UTILS_DATASTRUCTURE_ALGORITHM_H_
#include "Utils/Math/Math.h"

namespace our_graph::utils {
struct AABB {
  math::Vec3 min {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
  math::Vec3 max {0, 0, 0};
};

struct Triangle {
  math::Vec3 vertex0;
  math::Vec3 vertex1;
  math::Vec3 vertex2;
};

math::Vec3 QuickSearchN(const math::Vec3* data,
                          size_t size,
                          int axis_idx,
                          int n);

AABB ComputeAABB(const math::Vec3* data,
                 size_t size);
}

#endif //OUR_GRAPHIC_UTILS_DATASTRUCTURE_ALGORITHM_H_
