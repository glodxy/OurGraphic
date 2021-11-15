//
// Created by chaytian on 2021/11/12.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_RASTERIZERBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_RASTERIZERBASE_H_
#include <cstddef>
namespace our_graph {
struct Triangle;
struct Pixel;
class RasterizerBase {
 public:
  /**
   * 进行光栅化，目前仅针对三角形
   * @param triangles:三角形序列的首指针
   * @param size:三角形数量
   * @param pixel:输出的像素列表
   * @param pixel_size:输出对像素数量
   * */
  virtual void Rasterize(const Triangle* triangles, size_t size,
                         Pixel*& pixel, size_t& pixel_size) = 0;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_RASTERIZERBASE_H_
