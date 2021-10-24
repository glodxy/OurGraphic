//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_VULKANRENDERPROCESSOR_H_
#define OUR_GRAPHIC_FRAMEWORK_VULKANRENDERPROCESSOR_H_
#include "IRenderProcessor.h"
#include "Framework/Backend/include/Driver.h"
#include "ShaderCache.h"
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
  Driver* driver_{nullptr};
  SwapChainHandle sch_;
  RenderTargetHandle rth_;
  std::unique_ptr<ShaderCache> shader_cache_;
  uint64_t last_time = 0;
  int frame = 1;
  ShaderHandle rh_;
  RenderPrimitiveHandle rph_;
  PipelineState ps_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_VULKANRENDERPROCESSOR_H_
