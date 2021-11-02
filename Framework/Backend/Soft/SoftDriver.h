//
// Created by chaytian on 2021/11/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTDRIVER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTDRIVER_H_
#include "Backend/include/DriverApi.h"
class SDL_Window;


namespace our_graph {
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

  void Commit(SwapChainHandle handle) override;

  void Draw(PipelineState state, RenderPrimitiveHandle handle) override;

 private:
  int cur_width_{0}, cur_height_{0};
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTDRIVER_H_
