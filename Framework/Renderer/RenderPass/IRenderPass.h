//
// Created by Glodxy on 2022/1/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERPASS_IRENDERPASS_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERPASS_IRENDERPASS_H_

namespace our_graph::renderer {
class Driver;
/**
 * 该类描述了renderpass的基本动作
 * */
class IRenderPass {
 public:
  explicit IRenderPass(Driver* driver) : driver_(driver) {}
  /**
   * 装载函数，用于加载以及分配相关的资源
   * */
  virtual void Setup() = 0;
  /**
   * 执行函数，用于执行实际的渲染流程
   * */
  virtual void Execute() = 0;
 protected:
  Driver* driver_ {nullptr}
;};
}
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERPASS_IRENDERPASS_H_
