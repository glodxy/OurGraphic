//
// Created by 77205 on 2022/1/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_IRENDERER_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_IRENDERER_H_
#include <vector>
#include "Backend/include/Driver.h"
#include "RenderGraph/RenderGraph.h"
namespace our_graph {
// 该结构为每帧所需要的数据
class PerViewData;
// 该数据为每个物体的属性
class PerRenderableData {};
/**
 * 该类为所有renderer的基类，规定了renderer的通用能力
 * */
class IRenderer {
 public:
  explicit IRenderer(Driver* driver) : driver_(driver) {}

  virtual void Render() = 0;

 protected:
  Driver* driver_;
};

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_IRENDERER_H_
