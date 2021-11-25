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
#include "Framework/Resource/BufferBuilder.h"
#include "Manager/SystemManager.h"
#include "Manager/Entity.h"
#include "Component/Renderable.h"
#include "Component/Transform.h"
#include "Component/Camera.h"
namespace our_graph {
void VulkanRenderProcessor::Init() {
  driver_ = CreateDriver(Backend::VULKAN);
  shader_cache_ = std::make_unique<ShaderCache>();
  SystemManager::GetInstance().Init(driver_);
}

void VulkanRenderProcessor::Destroy() {
  SystemManager::GetInstance().Close();
  driver_->DestroySwapChain(sch_);

  DestroyDriver(driver_);
  driver_ = nullptr;
}

void VulkanRenderProcessor::End() {
}

void VulkanRenderProcessor::Start() {
  sch_ = driver_->CreateSwapChain(DriverContext::Get().window_handle_,
                                  uintptr_t(DriverContext::Get().sdl_window_));
  start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
  ).count();
  auto entity = Entity::Builder().Build();
  auto renderable = entity.AddComponent<Renderable>("monkey.obj");
  auto transform = entity.AddComponent<Transform>();
  transform->SetPosition({0, 0, 1});
  entity_id_ = entity.GetInstanceID();


  auto camera = Entity::Builder().Build();
  camera.AddComponent<Transform>();
  camera.AddComponent<Camera>();
}


void VulkanRenderProcessor::AfterRender() {
  driver_->Commit(sch_);
  driver_->EndFrame(frame++);
  driver_->Tick();
  FlushDriverCommand();

  auto entity = ENTITY_CAST(entity_id_);
  auto transform = entity.GetComponent<Transform>();
  transform->SetRotate({0, 2*frame, 0});
}

void VulkanRenderProcessor::BeforeRender() {
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
  current_time = float(last_time - start_time)/100.f;
  driver_->MakeCurrent(sch_, sch_);
  driver_->Tick();
  driver_->BeginFrame(time, frame);
  FlushDriverCommand();
}

void VulkanRenderProcessor::Render() {
  SystemManager::GetInstance().Update(frame);
  FlushDriverCommand();
}

}  // namespace our_graph