//
// Created by Glodxy on 2021/11/24.
//

#ifndef OUR_GRAPHIC__NATIVEWINDOWHELPER_H_
#define OUR_GRAPHIC__NATIVEWINDOWHELPER_H_

struct SDL_Window;

extern "C" void* GetNativeWindow(SDL_Window* sdlWindow);

#if defined(__APPLE__)
// Add a backing CAMetalLayer to the NSView and return the layer.
extern "C" void* SetUpMetalLayer(void* nativeWindow);
// Setup the window the way Filament expects (color space, etc.).
extern "C" void PrepareNativeWindow(SDL_Window* sdlWindow);
// Resize the backing CAMetalLayer's drawable to match the new view's size. Returns the layer.
extern "C" void* ResizeMetalLayer(void* nativeView);
#endif

#endif //OUR_GRAPHIC__NATIVEWINDOWHELPER_H_
