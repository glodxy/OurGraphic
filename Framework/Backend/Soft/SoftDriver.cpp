//
// Created by chaytian on 2021/11/2.
//

#include "SoftDriver.h"
#include "Backend/include_internal/Dispatcher.h"
#include "SoftSwapChain.h"
namespace our_graph {

SoftDriver::SoftDriver(SDL_Window *window) : DriverApi() {
  dispatcher_ = new Dispatcher<SoftDriver>();
  SoftContext::Get().window_ = window;
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  SoftContext::Get().window_width_ = w;
  SoftContext::Get().window_height_ = h;

  SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SoftContext::Get().renderer_ = renderer;
}

SoftDriver::~SoftDriver() {
  SDL_DestroyRenderer(SoftContext::Get().renderer_);
  SoftContext::Get().renderer_ = nullptr;
  SoftContext::Get().current_tex_ = nullptr;
  delete dispatcher_;
}

SwapChainHandle SoftDriver::CreateSwapChainS() {
  return AllocHandle<SoftSwapChain>();
}

void SoftDriver::CreateSwapChainR(SwapChainHandle handle, void *native_window, uint64_t flags) {
  Construct<SoftSwapChain>(handle);
}

void SoftDriver::Commit(SwapChainHandle handle) {
  auto swap_chain = HandleCast<SoftSwapChain*>(handle);
  swap_chain->CommitAndAcquireNext();
  // 判断resize
  int w, h;
  SDL_Window* window = SoftContext::Get().window_;
  SDL_GetWindowSize(window, &w, &h);
  SoftContext::Get().window_width_ = w;
  SoftContext::Get().window_height_ = h;
  if (w != cur_width_ || h != cur_height_) {
    swap_chain->Resize();
  }
  cur_width_ = w;
  cur_height_ = h;
  SoftContext::Get().current_tex_ = swap_chain->GetCurrent();
}

void SoftDriver::Draw(PipelineState state, RenderPrimitiveHandle handle) {

}

}