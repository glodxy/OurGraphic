//
// Created by chaytian on 2021/11/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTDRIVER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTDRIVER_H_
#include <memory>
#include "Backend/include/DriverApi.h"
#include "Backend/Soft/Base/SoftPipelineBase.h"
class SDL_Window;


namespace our_graph {
class SoftRenderTarget;
/**
 * 目前支持：
 * CreateSwapChain()
 * Draw()
 * Commit(SwapChain)
 * */
class SoftDriver : public DriverApi{
 public:
  /**
   * 使用sdl的window进行初始化，并将其加入到context
   * */
  explicit SoftDriver(SDL_Window* window);
  ~SoftDriver();

  SwapChainHandle CreateSwapChainS() override;
  void CreateSwapChainR(SwapChainHandle handle, void *native_window, uint64_t flags) override;

  RenderTargetHandle CreateDefaultRenderTargetS() override;
  void CreateDefaultRenderTargetR(RenderTargetHandle handle) override;

  void BeginRenderPass(RenderTargetHandle handle, const RenderPassParams &params) override;
  void EndRenderPass() override;

  void Commit(SwapChainHandle handle) override;

  void Draw(PipelineState state, RenderPrimitiveHandle handle) override;

 private:
  // 当前仅支持一条渲染管线
  std::unique_ptr<SoftPipelineBase> pipeline_;
  // 当前的render target
  SoftRenderTarget* current_rt_;
  int cur_width_{0}, cur_height_{0};
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTDRIVER_H_
