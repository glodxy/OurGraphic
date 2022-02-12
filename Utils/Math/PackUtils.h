//
// Created by Glodxy on 2022/2/12.
//

#ifndef OUR_GRAPHIC_UTILS_MATH_PACKUTILS_H_
#define OUR_GRAPHIC_UTILS_MATH_PACKUTILS_H_
#include "Math.h"
namespace our_graph::math {
class PackUtils {
 public:
  /**
   * 将normal与tangents打包成一个四元数
   * */
  static Vec4 PackTangentFrame(Vec3 normal, Vec3 tangents);
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_UTILS_MATH_PACKUTILS_H_
