//
// Created by chaytian on 2021/11/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTPIPELINE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTPIPELINE_H_
#include <climits>
#include "Base/SoftPipelineBase.h"
#include "Base/RasterizerBase.h"
#include "Base/TestBase.h"
#include "Base/ShaderBase.h"
#include "glm/glm.hpp"
namespace our_graph {
//struct Vertex {
//  union {
//    struct {
//      float x;
//      float y;
//      float z;
//    };
//    float data[3];
//    glm::vec3 vec;
//  };
//};
struct Vertex {
  Vec3 position;
};

struct Triangle {
  const Vertex * a;
  const Vertex * b;
  const Vertex * c;
};

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
  uint32_t x {0}; // 屏幕坐标x
  uint32_t y {0}; // 屏幕坐标y
  Color color {0}; // 颜色值
  float depth {std::numeric_limits<float>::max()};
  Pixel(){}
  Pixel(uint32_t px, uint32_t py) : x(px), y(py), color{0} {}
};

static_assert(sizeof(Color) == 4, "Color Size Not Match");
static_assert(sizeof(Pixel) == 16, "Pixel Size Not Match");

class SoftPipeline : public SoftPipelineBase {
 public:
  explicit SoftPipeline();
  void Execute(const Vertex *vertex, size_t size, void* context) override;

  void VertexShade(const Vertex *vertex, size_t size, Vertex *&dst_vertex, size_t &dst_size) override;
  bool GeometryTriangle(const Vertex* vertex, size_t size,
                        Triangle*& triangle, size_t& triangle_size) override;
  void Rasterize(const Triangle *triangles, size_t size, Pixel *&pixel, size_t &pixel_size) override;
  void Test(const Pixel *src_pixel, size_t src_size, Pixel *&dst_pixel, size_t &dst_size) override;
  void PixelShade(Pixel *pixel, size_t size) override;

  void DestroyPixel(Pixel *&pixel, size_t size) override;
  void DestroyTriangle(Triangle *&triangle, size_t size) override;
  void DestroyVertex(Vertex *&vertex, size_t size) override;

  void PixelBlit(const Pixel *pixel, size_t size, void* context) override;

 protected:
  void SingleVertexShade(const Vertex& src, Vec4& dst);

 private:
  bool enable_test_ {false};
  std::unique_ptr<RasterizerBase> rasterizer_; // 光栅器
  std::unique_ptr<TestBase> tester_; // 测试器
  std::unique_ptr<ShaderBase> shader_; // 着色器
  int frame_ = 0;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTPIPELINE_H_
