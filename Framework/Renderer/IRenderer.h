//
// Created by 77205 on 2022/1/26.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_IRENDERER_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_IRENDERER_H_
#include <vector>
#include "Backend/include/Driver.h"
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
  /**
   * 此处执行实际的逻辑，
   * 会调用render graph的相关函数，完成实际的渲染。
   * */
  virtual void Execute(const PerViewData& per_view,
                       const std::vector<PerRenderableData>& renderables);

 protected:
  Driver* driver_;
};

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_IRENDERER_H_
