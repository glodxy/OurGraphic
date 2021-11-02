//
// Created by chaytian on 2021/11/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTPIPELINE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTPIPELINE_H_
#include "Base/SoftPipelineBase.h"
namespace our_graph {
struct Color {
  union {
    struct {
      uint8_t r;
      uint8_t g;
      uint8_t b;
      uint8_t a;
    };
    uint32_t num;
  };
};

struct Pixel {
  const uint32_t x; // 屏幕坐标x
  const uint32_t y; // 屏幕坐标y
  Color color; // 颜色值
  Pixel(uint32_t px, uint32_t py) : x(px), y(py), color{0} {}
};
static_assert(sizeof(Color) == 4, "Color Size Not Match");
static_assert(sizeof(Pixel) == 12, "Pixel Size Not Match");

class SoftPipeline : public SoftPipelineBase {
 public:
  void DestroyPixel(Pixel *&pixel, size_t size) override;
  void DestroyTriangle(Triangle *&triangle, size_t size) override;
  void DestroyVertex(Vertex *&vertex, size_t size) override;

  void PixelBlit(const Pixel *pixel, size_t size) override;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTPIPELINE_H_
