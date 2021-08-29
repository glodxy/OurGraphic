//
// Created by Glodxy on 2021/8/25.
//

#include "SDL2/SDL.h"
#include <iostream>
#include "Utils/OGLogging.h"
#include "Framework/VulkanRenderProcessor.h"

namespace {
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
}

bool Init(SDL_Window*& window, SDL_Renderer*& renderer) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout<<"sdl error: "<< SDL_GetError()<<std::endl;
    return false;
  }
  window = SDL_CreateWindow("hello, sdl!", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                            SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) {
    std::cout<<"sdl window error: "<< SDL_GetError()<<std::endl;
    return false;
  }
  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    std::cout<<"sdl renderer error:"<<SDL_GetError()<<std::endl;
    return false;
  }
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
  return true;
}

int main(int argc, char** argv) {
  our_graph::InitLogger("og_logger");
  our_graph::LOG_INFO("main", "test:{}", "init finished");

  SDL_Window *window = nullptr;
  SDL_Renderer* renderer = nullptr;
  std::shared_ptr<our_graph::IRenderProcessor> render_engine =
      our_graph::IRenderProcessor::GetInstance<our_graph::VulkanRenderProcessor>();
  if (!Init(window, renderer)) {
    std::cerr<<"创建失败"<<std::endl;
    return -1;
  }
  render_engine->Init();
  render_engine->Start();
  SDL_Event event;
  bool quit = false;
  do {
    SDL_WaitEvent(&event);
    switch (event.type) {
      case SDL_QUIT: {
        quit = true;
        break;
      }
      default: {
        break;
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
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }

  return 0;
}