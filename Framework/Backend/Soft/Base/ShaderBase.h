//
// Created by Glodxy on 2021/11/15.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_SHADERBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_SHADERBASE_H_
#include "ModuleBase.h"
namespace our_graph {
struct Vertex;
struct Pixel;
class ShaderBase : public ModuleBase {
 public:
  virtual void VertexShade(const Vertex* in, Vertex* out) = 0;
  virtual void PixelShade(const Pixel* in, Pixel* out) = 0;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_SHADERBASE_H_
