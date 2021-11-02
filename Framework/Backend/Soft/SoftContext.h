//
// Created by chaytian on 2021/11/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTCONTEXT_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTCONTEXT_H_
#include "SDL2/SDL.h"

namespace our_graph {

class SoftContext {
 public:
  static SoftContext& Get() {
    static SoftContext context_;
    return context_;
  }
  SDL_Window * window_ {nullptr};
  SDL_Renderer * renderer_ {nullptr};
  SDL_Texture * current_tex_ {nullptr};
  uint32_t window_width_ {0};
  uint32_t window_height_ {0};
 private:
  SoftContext() = default;

};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTCONTEXT_H_
