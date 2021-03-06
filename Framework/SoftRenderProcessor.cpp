//
// Created by chaytian on 2021/11/3.
//

#include "SoftRenderProcessor.h"
#include "DriverContext.h"
#include "Framework/Resource/BufferBuilder.h"
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
  LOG_INFO("SoftRenderer", "End Render Engine!");
}

void SoftRenderProcessor::Start() {
  sch_ = driver_->CreateSwapChain(DriverContext::Get().window_handle_,
                                  uintptr_t(DriverContext::Get().sdl_window_));
  rth_ = driver_->CreateDefaultRenderTarget();
  rph_ = driver_->CreateRenderPrimitive();
  auto vertex = BufferBuilder::BuildDefaultVertex(driver_);
  auto index = BufferBuilder::BuildDefaultIndex(driver_);
  driver_->SetRenderPrimitiveBuffer(rph_, vertex->GetHandle(), index->GetHandle());
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
  int cnt = 0;
  do {
    time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    std::this_thread::sleep_for(std::chrono::milliseconds (1));
    ++cnt;
  } while((time - last_time) < target_time);
  LOG_INFO("SoftRenderer", "Wait {} ms for 60fps, frame:{}", cnt, frame++);
  float f = (1000.f) / (time - last_time);
  last_time = time;
  driver_->MakeCurrent(sch_, sch_);
  driver_->Tick();
  FlushDriverCommand();
}

void SoftRenderProcessor::Render() {
  driver_->BeginRenderPass(rth_, RenderPassParams());
  driver_->Draw(ps_, rph_);
  driver_->EndRenderPass();
  FlushDriverCommand();
}

}