//
// Created by chaytian on 2021/11/3.
//

#include "SoftRenderProcessor.h"
#include "DriverContext.h"
#include "SDL2/SDL.h"
namespace our_graph {
void SoftRenderProcessor::Init() {
  driver_ = CreateDriver(Backend::SOFT, DriverContext::Get().sdl_window_);
}

void SoftRenderProcessor::Destroy() {
  driver_->DestroySwapChain(sch_);

  DestroyDriver(driver_);
  driver_ = nullptr;
}

void SoftRenderProcessor::End() {
}

void SoftRenderProcessor::Start() {
  sch_ = driver_->CreateSwapChain(DriverContext::Get().window_handle_,
                                  uintptr_t(DriverContext::Get().sdl_window_));

  start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
  ).count();
}


void SoftRenderProcessor::AfterRender() {
  driver_->Commit(sch_);
  driver_->Tick();
  FlushDriverCommand();
}

void SoftRenderProcessor::BeforeRender() {
  // 控制帧数
  uint64_t time;
  float target_time = 1000.f / 60.f;
  do {
    time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    std::this_thread::sleep_for(std::chrono::milliseconds (1));
  } while((time - last_time) < target_time);

  float f = (1000.f) / (time - last_time);
  last_time = time;
  driver_->MakeCurrent(sch_, sch_);
  driver_->Tick();
  FlushDriverCommand();
}

void SoftRenderProcessor::Render() {
  driver_->Draw(ps_, rph_);
  FlushDriverCommand();
}

}