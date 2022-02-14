//
// Created by Glodxy on 2022/2/14.
//

#ifndef OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPUTILS_H_
#define OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPUTILS_H_
#include "Cubemap.h"
namespace our_graph::image {
class CubemapUtils {
 public:
  static Cubemap CreateCubemap(size_t dim);

  //! 降采样
  static void DownsampleCubemapLevelBoxFilter(Cubemap& dst, const Cubemap& src);

  //! 全景图--->cubemap
  static void EquirectangularToCubemap(Cubemap& dst, const LinearImage& src);
};
}  // namespace our_graph::image
#endif //OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPUTILS_H_
