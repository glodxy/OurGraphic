//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_VULKANRENDERPROCESSOR_H_
#define OUR_GRAPHIC_FRAMEWORK_VULKANRENDERPROCESSOR_H_
#include "IRenderProcessor.h"
#include "Framework/Backend/include/Driver.h"

namespace our_graph {
class VulkanRenderProcessor : public IRenderProcessor {
  friend class IRenderEngine;
 public:
  explicit VulkanRenderProcessor(token){}

  void Init() override;
  void Destroy() override;
 protected:
  void Start() override;
  void End() override;

  void BeforeRender() override;
  void Render() override;
  void AfterRender() override;

 private:
  DriverApi* driver_{nullptr};
  SwapChainHandle sch_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_VULKANRENDERPROCESSOR_H_
