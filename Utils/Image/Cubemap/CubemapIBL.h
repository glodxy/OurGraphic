//
// Created by Glodxy on 2022/2/14.
//

#ifndef OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPIBL_H_
#define OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPIBL_H_
#include "Cubemap.h"
namespace our_graph::image {
class CubemapIBL {
 public:
  static void DiffuseIrradiance(Cubemap& dst, const std::vector<Cubemap>& levels,
                                size_t maxNumSamples = 1024);
};
}  // namespace our_graph::image
#endif //OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPIBL_H_
