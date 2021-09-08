//
// Created by Glodxy on 2021/9/8.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_ISWAPCHAIN_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_ISWAPCHAIN_H_
#include <memory>
#include "IRenderDevice.h"
#include "ITexture.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace our_graph {
  using WindowInstance = HINSTANCE;
  using WindowHandle = HWND;
class ISwapChain {
 public:
  virtual void Create(WindowInstance ins, WindowHandle handle) = 0;
  virtual void Destroy() = 0;

  virtual int GetRenderTargetCnt() const = 0;
  virtual std::shared_ptr<ITexture> GetRenderTarget(int idx) = 0;
 protected:

};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_ISWAPCHAIN_H_
