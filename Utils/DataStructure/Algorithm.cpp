//
// Created by Glodxy on 2021/12/18.
//

#include "Algorithm.h"
#include "Utils/OGLogging.h"
namespace our_graph::utils {
using namespace our_graph::math;
Vec3 InterQuickSearchN(const Vec3* data,
                        int begin,
                        int end,
                        int axis_idx,
                        int n) {
  if (end == begin) {
    return data[begin];
  }
  if (n > (end - begin + 1)) {
    LOG_ERROR("QuickSearchN", "target[{}] > size:{}",
              n, size);
    return Vec3(0, 0, 0);
  }
  if (axis_idx > 2 || axis_idx < 0) {
    LOG_ERROR("QuickSearchN", "axis_idx:{} not supported!");
    return Vec3(0, 0, 0);
  }
  Vec3 key = data[begin];
  int i = begin;
  int j = end;
  while (i < j) {
    while (i < j && data[j][axis_idx] > key[axis_idx]) {
      --j;
    }
    if (i < j) {
      data[i] = data[j];
      ++i;
    }
    while (i < j && data[i][axis_idx] < key[axis_idx]) {
      ++i;
    }
    if (i < j) {
      data[j] = data[i];
      --j;
    }
  }
  data[i] = key;
  int middle_num = i - begin;
  if (middle_num == n) {
    return data[i];
  } else if (middle_num < n) {
    return InterQuickSearchN(data, i+1, end, axis_idx, n - middle_num - 1);
  } else {
    return InterQuickSearchN(data, begin, i-1, axis_idx, n);
  }
}

AABB ComputeAABB(const math::Vec3* data,
                 size_t size) {
  AABB box;
  for (int i = 0; i < size; ++i) {
    box.min[0] = std::min(box.min[0], data[i][0]);
    box.min[1] = std::min(box.min[1], data[i][1]);
    box.min[2] = std::min(box.min[2], data[i][2]);

    box.max[0] = std::max(box.max[0], data[i][0]);
    box.max[1] = std::max(box.max[1], data[i][1]);
    box.max[2] = std::max(box.max[2], data[i][2]);
  }
  return box;
}
}