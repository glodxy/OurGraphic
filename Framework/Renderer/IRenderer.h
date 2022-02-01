//
// Created by glodxy on 2022/1/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_IRENDERER_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_IRENDERER_H_
#include <vector>
#include "Backend/include/Driver.h"
namespace our_graph {
/**
 * 该类为所有renderer的基类，规定了renderer的通用能力
 * */
class IRenderer {
 public:
  explicit IRenderer(Driver* driver) : driver_(driver) {}

  virtual void Init() = 0;
  virtual void Render() = 0;

  virtual void Update(uint32_t time) = 0;

  virtual void Reset(void* params) = 0;

  virtual void Destroy() = 0;
 protected:
  Driver* driver_;
};

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_IRENDERER_H_
