//
// Created by chaytian on 2021/11/2.
//

#include "SoftPipeline.h"
#include "SoftContext.h"
#include "SDL2/SDL.h"

namespace our_graph {

/**
 * 此处假设x为列，y为行
 * */
void SoftPipeline::PixelBlit(const Pixel *pixel, size_t size) {
  SDL_Texture* current_texture = SoftContext::Get().current_tex_;
  assert(current_texture);

  void* tex;
  int pitch;

  SDL_LockTexture(current_texture, nullptr, &tex, &pitch);
  for (int i = 0; i < size; ++i) {
    const Pixel p = pixel[i];
    uint32_t idx = p.y * (pitch/4) + p.x;
    ((uint32_t*)tex)[idx] = p.color.num;
  }
  
  SDL_UnlockTexture(current_texture);
}

}