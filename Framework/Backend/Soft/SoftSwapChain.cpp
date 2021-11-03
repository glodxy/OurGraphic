//
// Created by chaytian on 2021/11/2.
//

#include "SoftSwapChain.h"
#include <cassert>
#include "Utils/OGLogging.h"

namespace our_graph {

SoftSwapChain::SoftSwapChain() {
  Create();
}

SoftSwapChain::~SoftSwapChain() noexcept {
  Destroy();
}

SDL_Texture *const SoftSwapChain::GetCurrent() const {
  assert(current_idx_ < SWAPCHAIN_TEX_SIZE);
  return textures_[current_idx_];
}

void SoftSwapChain::CommitAndAcquireNext() {
  SDL_Renderer * renderer = SoftContext::Get().renderer_;
  SDL_Texture* tex = SoftContext::Get().current_tex_;
  if (!renderer) {
    LOG_ERROR("SoftSwapChain", "Commit Failed! No Renderer!");
    return;
  }
  // 拷贝纹理
  if (SDL_RenderCopy(renderer, tex, nullptr, nullptr) != 0) {
    LOG_ERROR("SoftSwapchain", "Copy Tex Failed!");
  }
  // 展示
  SDL_RenderPresent(renderer);
  current_idx_ = (current_idx_ + 1) % SWAPCHAIN_TEX_SIZE;
  LOG_INFO("SoftSwapchain", "Presented!");
}

void SoftSwapChain::Destroy() {
  for (int i = 0; i < SWAPCHAIN_TEX_SIZE; ++i) {
    SDL_DestroyTexture(textures_[i]);
  }
  current_idx_ = 0;
}

void SoftSwapChain::Create() {
  SDL_Renderer* renderer = SoftContext::Get().renderer_;
  uint32_t width = SoftContext::Get().window_width_;
  uint32_t height = SoftContext::Get().window_height_;
  assert(width != 0 && height != 0);
  if (!renderer) {
    LOG_ERROR("SoftSwapChain", "Create Failed! No Renderer!");
    assert(!renderer);
    return;
  }
  for (int i = 0; i < SWAPCHAIN_TEX_SIZE; ++i) {
    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_STREAMING, width, height);
    textures_[i] = tex;
  }
  SoftContext::Get().current_tex_ = textures_[current_idx_];
}

void SoftSwapChain::Resize() {
  Destroy();
  Create();
}

}