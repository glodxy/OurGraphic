//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanRenderProcessor.h"

#include "Backend/Vulkan/VulkanDriver.h"
#include "DriverContext.h"
#if __APPLE__
#include "Backend/Vulkan/VulkanPlatformMacos.h"
#elif WIN32
#include "Backend/Vulkan/VulkanPlatformWindows.h"
#endif
#include "Backend/Vulkan/VulkanSwapChain.h"
#include "Backend/include/PipelineState.h"
namespace our_graph {
void VulkanRenderProcessor::Init() {
  std::unique_ptr<IPlatform> platform;
#if __APPLE__
  platform = std::make_unique<VulkanPlatformMacos>();
#elif WIN32
  platform = std::make_unique<VulkanPlatformWindows>();
#endif
  driver_ = new VulkanDriver();
  driver_->Init(std::move(platform));
  shader_cache_ = std::make_unique<ShaderCache>();
 }

void VulkanRenderProcessor::Destroy() {
  driver_->DestroyRenderTarget(rth_);
  driver_->DestroySwapChain(sch_);

  driver_->Clear();
  delete driver_;
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
  auto vertex = driver_->CreateVertexBuffer(0, 0, 0, {});
  auto index = driver_->CreateIndexBuffer(ElementType::UINT, 0, BufferUsage::DYNAMIC);
  //driver_->SetRenderPrimitiveBuffer(rph_, vertex, index);
  driver_->SetRenderPrimitiveRange(rph_, PrimitiveType::TRIANGLES, 0, 0, 0, 0);
  ps_.shader_ = rh_;
  ps_.raster_state_.depthFunc = SamplerCompareFunc::GE;
}


void VulkanRenderProcessor::AfterRender() {
  driver_->Commit(sch_);

  driver_->EndFrame(frame++);
  driver_->Tick();
}

void VulkanRenderProcessor::BeforeRender() {
  driver_->MakeCurrent(sch_, sch_);
  driver_->Tick();
  uint64_t time = std::chrono::system_clock::now().time_since_epoch().count();
  driver_->BeginFrame(time, frame);
}

void VulkanRenderProcessor::Render() {
  RenderPassParams params;
  params.clearColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
  params.flags.clear = TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH;
  params.viewport.width = 1920;
  params.viewport.height = 1080;
  driver_->BeginRenderPass(rth_, params);
  driver_->Draw(ps_, rph_);
  driver_->EndRenderPass();
}

}  // namespace our_graph