//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_VULKANRENDERPROCESSOR_H_
#define OUR_GRAPHIC_FRAMEWORK_VULKANRENDERPROCESSOR_H_
#include "IRenderProcessor.h"
#include "Framework/Backend/include/IRenderInstance.h"
#include "Framework/Backend/include/IRenderDevice.h"
#include "Framework/Backend/include_internal/ICommandPool.h"
#include "Framework/Backend/include/ITexture.h"
#include "Framework/Backend/include/ISwapchain.h"

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
  std::shared_ptr<IRenderInstance> render_instance_;
  std::shared_ptr<IRenderDevice> render_device_;
  std::shared_ptr<ICommandPool> command_buffer_;
  std::shared_ptr<ITexture> texture_;
  std::shared_ptr<ISwapChain> swapchain_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_VULKANRENDERPROCESSOR_H_
