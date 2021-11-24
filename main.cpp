//
// Created by Glodxy on 2021/8/25.
//

#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include <iostream>
#include <SDL2/SDL_syswm.h>
#include "Utils/OGLogging.h"
#include "Framework/DriverContext.h"
#include "Framework/VulkanRenderProcessor.h"
#include "Framework/SoftRenderProcessor.h"
#include "NativeWindowHelper.h"

namespace {
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
}


void Resize(SDL_Window* window) {
  void* native_window = ::GetNativeWindow(window);
#if __APPLE__
  ::ResizeMetalLayer(native_window);
#endif
}


bool Init(SDL_Window*& window) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout<<"sdl error: "<< SDL_GetError()<<std::endl;
    return false;
  }
  window = SDL_CreateWindow("hello, sdl!", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                            SCREEN_HEIGHT, SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
  if (!window) {
    std::cout<<"sdl window error: "<< SDL_GetError()<<std::endl;
    return false;
  }
  return true;
}

int main(int argc, char** argv) {
  our_graph::InitLogger("og_logger");
  our_graph::LOG_INFO("main", "test:{}", "init finished");

  SDL_Window *window = nullptr;
  std::shared_ptr<our_graph::IRenderProcessor> render_engine =
      our_graph::IRenderProcessor::GetInstance<our_graph::VulkanRenderProcessor>();
  if (!Init(window)) {
    std::cerr<<"创建失败"<<std::endl;
    return -1;
  }
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(window, &wmInfo);
  our_graph::DriverContext::Get().sdl_window_ = window;
  void* native_window = ::GetNativeWindow(window);
#if __APPLE__
  ::PrepareNativeWindow(window);
  ::SetUpMetalLayer(native_window);
#endif
  our_graph::DriverContext::Get().window_handle_ = native_window;
  our_graph::DriverContext::Get().window_width_ = SCREEN_WIDTH;
  our_graph::DriverContext::Get().window_height_ = SCREEN_HEIGHT;
  render_engine->Init();
  render_engine->Start();
  SDL_Event event;
  bool quit = false;
  do {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
          break;
        }
        case SDL_WINDOWEVENT:
          switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              Resize(window);
              break;
            default:
              break;
          }
          break;
        default: {
          break;
        }
      }
    }
    //渲染
    render_engine->BeforeRender();
    render_engine->Render();
    render_engine->AfterRender();
  } while (!quit);
  render_engine->End();
  render_engine->Destroy();
  if (window) {
    SDL_DestroyWindow(window);
  }
  spdlog::drop_all();
  return 0;
}