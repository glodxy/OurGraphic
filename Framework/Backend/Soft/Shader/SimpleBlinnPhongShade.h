//
// Created by Glodxy on 2021/11/15.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SHADER_SIMPLEBLINNPHONGSHADE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SHADER_SIMPLEBLINNPHONGSHADE_H_
#include "Backend/Soft/Base/ShaderBase.h"
namespace our_graph {
class SimpleBlinnPhongShade : public ShaderBase {
 public:
  void VertexShade(const Vertex *in, Vertex *out) override;
  void PixelShade(const Pixel *in, Pixel *out) override;
};
}  // namespace out_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SHADER_SIMPLEBLINNPHONGSHADE_H_
