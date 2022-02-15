//
// Created by Glodxy on 2022/2/14.
//

#ifndef OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPIBL_H_
#define OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPIBL_H_
#include <vector>
#include "Cubemap.h"
namespace our_graph::image {
class CubemapIBL {
 public:
  /**
   * 采样Cubemap得到对应的irradiance贴图
   * @param dst:得到第Cubemap
   * @param levels：不同等级的cubemap
   * @param max_sample_nums：用于重要性采样的最大采样数
   * */
  static void DiffuseIrradiance(Cubemap& dst, const std::vector<Cubemap>& levels,
                                size_t max_sample_nums = 1024);
};
}  // namespace our_graph::image
#endif //OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPIBL_H_
