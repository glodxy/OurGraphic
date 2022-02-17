//
// Created by Glodxy on 2022/2/14.
//

#ifndef OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPUTILS_H_
#define OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPUTILS_H_
#include "Cubemap.h"
namespace our_graph::image {
class CubemapUtils {
 public:
  static Cubemap CreateCubemap(LinearImage& image, size_t dim);


  /**
   * 将image的数据设置到dst的一个面上
   * */
  static void SetFaceFromSingleImage(Cubemap& dst, Cubemap::Face face, const LinearImage& image);

  /**
   *Hammersley采样
   * @param i：第i个采样点
   * @param iN：1.0f/总采样点数
   * */
  static math::Vec2 Hammersley(uint32_t i, float iN);

  //! 生成mipmap,输入为只有一个元素的levels
  static void GenerateMipmaps(std::vector<Cubemap>& levels);

  //! 降采样
  static void DownsampleCubemapLevelBoxFilter(Cubemap& dst, const Cubemap& src);

  //! 全景图--->cubemap
  static void EquirectangularToCubemap(Cubemap& dst, const LinearImage& src);

  //! 十字图--->cubemap
  static void CrossToCubemap(Cubemap& dst, const LinearImage& src);
 private:
  /**
   * 从image初始化dst的各个面（即绑定image的subset至各面)
   * @param image:所有面构成的image，该image会存储该cubemap的实际数据
   * */
  static void BindFaceFromWholeImage(Cubemap& dst, Cubemap::Face face, const LinearImage& image);
  // 创建cubemap的image资源，包括了所有的面
  static LinearImage CreateCubemapImage(size_t dim);
};
}  // namespace our_graph::image
#endif //OUR_GRAPHIC_UTILS_IMAGE_CUBEMAP_CUBEMAPUTILS_H_
