//
// Created by chaytian on 2021/11/2.
//

#include "SoftPlatformSDL.h"
#include "SDL2/SDL.h"
namespace our_graph {

bool SoftPlatformSDL::CreateSurface(void *native_window, void *instance, uint64_t flags, void *surface) {
  uintptr_t uintptr = uintptr_t(flags);
  SDL_Window* sdl_window = (SDL_Window*)uintptr;
  SDL_Surface * sdl_surface = SDL_GetWindowSurface(sdl_window);
  intptr_t* surface_addr = (intptr_t*) surface;
  *surface_addr = intptr_t(sdl_surface);
  return true;
}

}