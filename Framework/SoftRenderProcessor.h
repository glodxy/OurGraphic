//
// Created by chaytian on 2021/11/3.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_SOFTRENDERPROCESSOR_H_
#define OUR_GRAPHIC_FRAMEWORK_SOFTRENDERPROCESSOR_H_
#include "IRenderProcessor.h"
#include "Backend/include/Driver.h"
namespace our_graph {
class SoftRenderProcessor : public IRenderProcessor {
 public:
  explicit SoftRenderProcessor(token){}

  void Init() override;
  void Destroy() override;
 protected:
  void Start() override;
  void End() override;

  void BeforeRender() override;
  void Render() override;
  void AfterRender() override;

 private:
  Driver* driver_{nullptr};
  SwapChainHandle sch_;
  PipelineState ps_;
  RenderPrimitiveHandle rph_;
  uint64_t last_time = 0;
  uint64_t start_time = 0;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_SOFTRENDERPROCESSOR_H_
