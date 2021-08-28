//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_TESTRENDERENGINE_H_
#define OUR_GRAPHIC_FRAMEWORK_TESTRENDERENGINE_H_
#include "IRenderEngine.h"
#include "Backend/IRenderInstance.h"

namespace our_graph {
class TestRenderEngine : public IRenderEngine {
  friend class IRenderEngine;
 public:
  explicit TestRenderEngine(token){}

  void Init() override;
  void Destroy() override;
 protected:
  void Start() override;
  void End() override;

  void BeforeRender() override;
  void Render() override;
  void AfterRender() override;

 private:
  std::shared_ptr<IRenderInstance> render_instance_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_TESTRENDERENGINE_H_
