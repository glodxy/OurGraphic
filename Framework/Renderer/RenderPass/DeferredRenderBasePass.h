//
// Created by Glodxy on 2022/1/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERPASS_DEFERREDRENDERBASEPASS_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERPASS_DEFERREDRENDERBASEPASS_H_
#include "IRenderPass.h"
#include "Backend/include/Handle.h"
namespace our_graph::renderer {
/**
 * 该类描述了deferred render中的base pass
 * 即用于写入gbuffer的pass
 * input:gbuffer render_target
 *      visible renderable
 *      camera
 * */
class DeferredRenderBasePass : public IRenderPass {
 public:
  explicit DeferredRenderBasePass(Driver* driver);
  void Setup() override;
  void Execute() override;
 protected:

};
} // namespace our_graph::renderer
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERPASS_DEFERREDRENDERBASEPASS_H_
