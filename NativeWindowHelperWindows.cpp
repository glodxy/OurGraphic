//
// Created by Glodxy on 2021/11/24.
//
#include "NativeWindowHelper.h"
#include "SDL2/SDL_syswm.h"

void* GetNativeWindow(SDL_Window* sdlWindow) {
  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  SDL_GetWindowWMInfo(sdlWindow, &wmi);
  HWND win = (HWND) wmi.info.win.window;
  return (void*) win;
}
