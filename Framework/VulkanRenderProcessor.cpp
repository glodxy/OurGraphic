//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanRenderProcessor.h"

#include "Backend/Vulkan/VulkanDriver.h"
#include "DriverContext.h"
#include "Utils/OGLogging.h"
#if __APPLE__
#include "Backend/Vulkan/VulkanPlatformMacos.h"
#elif WIN32
#include "Backend/Vulkan/VulkanPlatformWindows.h"
#endif
#include "Backend/Vulkan/VulkanSwapChain.h"
#include "Backend/include/PipelineState.h"
#include "BufferBuilder.h"
namespace our_graph {
void VulkanRenderProcessor::Init() {
  driver_ = CreateDriver(Backend::VULKAN);
  shader_cache_ = std::make_unique<ShaderCache>();
 }

void VulkanRenderProcessor::Destroy() {
  driver_->DestroyRenderTarget(rth_);
  driver_->DestroySwapChain(sch_);

  DestroyDriver(driver_);
  driver_ = nullptr;
}

void VulkanRenderProcessor::End() {
  driver_->DestroyShader(rh_);
}

void VulkanRenderProcessor::Start() {
  sch_ = driver_->CreateSwapChain(DriverContext::Get().window_handle_, 0);
  rth_ = driver_->CreateDefaultRenderTarget();
  ShaderCache::ShaderBuilder builder = ShaderCache::ShaderBuilder("test_shader", shader_cache_.get());
  Program program = builder.Vertex("test.vert").Frag("test.frag").Build();
  rh_ = driver_->CreateShader(std::move(program));
  rph_ = driver_->CreateRenderPrimitive();

  auto vertex = BufferBuilder::BuildDefaultVertex(driver_);
  auto index = BufferBuilder::BuildDefaultIndex(driver_);
  float w = DriverContext::Get().window_width_;
  float h = DriverContext::Get().window_height_;
  auto resolution = BufferBuilder::BuildDefaultQuadUniformBuffer(
      driver_, w, h);
//  driver_->SetRenderPrimitiveBuffer(rph_, vertex->GetHandle(), index->GetHandle());
//  driver_->SetRenderPrimitiveRange(rph_, PrimitiveType::TRIANGLES, 0, 0, 0, index->GetIndexCount());
  ps_.shader_ = rh_;
  ps_.raster_state_.colorWrite = true;
  ps_.raster_state_.culling = CullingMode::NONE;
  driver_->BindUniformBuffer(0, resolution->GetHandle());
}


void VulkanRenderProcessor::AfterRender() {
  driver_->Commit(sch_);

  driver_->EndFrame(frame++);
  driver_->Tick();
  FlushDriverCommand();
}

void VulkanRenderProcessor::BeforeRender() {
  driver_->MakeCurrent(sch_, sch_);
  driver_->Tick();
  // 控制帧数
  uint64_t time;
  float target_time = 1000.f / 60.f;
  do {
    time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    Sleep(1);
  } while((time - last_time) < target_time);
  float f = (1000.f) / (time - last_time);
  last_time = time;
  LOG_ERROR("Frame", "frame:{}, fps:{}", frame, f);
  driver_->BeginFrame(time, frame);
  FlushDriverCommand();
}

void VulkanRenderProcessor::Render() {
  RenderPassParams params;
  params.clearColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
  params.flags.clear = TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH;
  params.viewport.width = 800;
  params.viewport.height = 600;
  params.viewport.left = 0;
  params.viewport.bottom = 0;
  params.clearDepth = 1.0f;
  driver_->BeginRenderPass(rth_, std::move(params));
  driver_->Draw(ps_, rph_);
  driver_->EndRenderPass();
  FlushDriverCommand();
}

}  // namespace our_graph