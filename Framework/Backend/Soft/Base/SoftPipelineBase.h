//
// Created by chaytian on 2021/11/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_SOFPIPELINEBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_SOFPIPELINEBASE_H_
#include <cstddef>
#include "Utils/OGLogging.h"

namespace our_graph {
struct Vertex;
struct Triangle;
struct Pixel;
class SoftPipelineBase {

 public:
  /**
   * 执行渲染管线，目前仅接受vertex的输入
   * todo：添加顶点的缓存，来进行顶点的复用
   * todo：考虑data的输入，并可配置数据读取方式
   * */
  void Execute(const Vertex* vertex, size_t size) {
    Vertex* transformed_vertex = nullptr;
    size_t transformed_vertex_cnt;
    VertexShade(vertex, size, transformed_vertex, transformed_vertex_cnt);

    Triangle* triangles = nullptr;
    size_t triangle_cnt;
    if (!GeometryTriangle(transformed_vertex, transformed_vertex_cnt,
                          triangles, triangle_cnt)) {
      LOG_ERROR("Pipeline", "Execute Failed! geometry failed!");
      return;
    }

    Pixel* src_pixels;
    size_t src_pixel_cnt;
    Rasterize(triangles, triangle_cnt,
              src_pixels, src_pixel_cnt);

    Pixel* tested_pixels;
    size_t tested_pixel_cnt;
    Test(src_pixels, src_pixel_cnt,
         tested_pixels, tested_pixel_cnt);

    // 此处只需改变颜色了，所以直接复用
    PixelShade(tested_pixels, tested_pixel_cnt);

    PixelBlit(tested_pixels, tested_pixel_cnt);

    //清除临时生成的资源
    DestroyPixel(tested_pixels, tested_pixel_cnt);
    DestroyPixel(src_pixels, src_pixel_cnt);
    DestroyTriangle(triangles, triangle_cnt);
    DestroyVertex(transformed_vertex, transformed_vertex_cnt);
  }

 protected:
  //todo：添加输入装配环节，方便顶点的复用

  /**
   * 顶点着色，对每一个顶点进行变换
   * @param vertex:顶点序列的首指针
   * @param size:顶点的数目
   *
   * @param dst_vertex:输出的顶点序列
   * @param dst_size:输出的顶点数目
   * */
  virtual void VertexShade(const Vertex* vertex, size_t size,
                           Vertex*& dst_vertex, size_t& dst_size) {}

  /**
   * 将顶点组装为几何，目前仅支持三角形
   *
   * @param vertex：输入的顶点序列
   * @param size：输入顶点数目
   * @param triangle：输出的三角序列
   * @param triangle_size：输出的三角数目
   * */
  bool GeometryTriangle(const Vertex* vertex, size_t size,
                        Triangle*& triangle, size_t& triangle_size) {
    return true;
  }
  /**
   * 进行光栅化，目前仅针对三角形
   * @param triangles:三角形序列的首指针
   * @param size:三角形数量
   * @param pixel:输出的像素列表
   * @param pixel_size:输出对像素数量
   * */
  virtual void Rasterize(const Triangle* triangles, size_t size,
                         Pixel*& pixel, size_t& pixel_size) {}

  /**
   * 进行各类测试，目前仅考虑深度测试
   * @param src_pixel:输入像素序列
   * @param src_size:输入像素的数目
   * @param dst_pixel:输出的像素序列
   * @param dst_size:输出的像素数目
   * */
  virtual void Test(const Pixel* src_pixel, size_t src_size,
                    Pixel*& dst_pixel, size_t& dst_size) {}
  /**
   * 像素着色，对像素序列进行逐个着色
   * @param pixel:像素序列的首指针
   * @param size：像素个数
   * */
  virtual void PixelShade(Pixel* pixel, size_t size) {}

  /**
   * 将像素序列blit到目标的rendertarget
   * 该函数必须被实现
   *
   * @param pixel：要blit的像素序列
   * @param size：像素数目
   * */
  virtual void PixelBlit(const Pixel* pixel, size_t size) = 0;


  /**
   * 销毁相关资源
   * 该函数会在一次管线执行结束后被调用
   * */
  virtual void DestroyVertex(Vertex*& vertex, size_t size) = 0;
  virtual void DestroyTriangle(Triangle*& triangle, size_t size) = 0;
  virtual void DestroyPixel(Pixel*& pixel, size_t size) = 0;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_SOFPIPELINEBASE_H_
