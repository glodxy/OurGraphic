//
// Created by chaytian on 2021/11/12.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SIMPLESOFTRASTERIZER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SIMPLESOFTRASTERIZER_H_
#include "Backend/Soft/Base/RasterizerBase.h"
#include "SoftPipeline.h"
namespace our_graph {
/**
 * todo:添加MSAA
 * */
class SimpleSoftRasterizer : public RasterizerBase {
 public:
  void Rasterize(const Triangle *triangles, size_t size, Pixel *&pixel, size_t &pixel_size) override;

 private:
  void RasterizerSingleTriangle(const Triangle& src, std::vector<Pixel>& pixels);
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SIMPLESOFTRASTERIZER_H_
