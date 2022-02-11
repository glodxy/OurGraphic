//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanRenderProcessor.h"

#include "Backend/Vulkan/VulkanDriver.h"
#include "DriverContext.h"
#include "Utils/OGLogging.h"
#include "include/GlobalEnum.h"
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
#include "Component/LightSource.h"
#include "Component/SkySource.h"
namespace our_graph {
uint32_t RenderContext::WIDTH = 0;
uint32_t RenderContext::HEIGHT = 0;
uint32_t RenderContext::TIME_MS = 0;

Driver* IRenderProcessor::driver_ = nullptr;

void VulkanRenderProcessor::Init() {
  driver_ = CreateDriver(Backend::VULKAN);
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
  auto transform = entity.AddComponent<Transform>();
  auto renderable = entity.AddComponent<Renderable>("monkey.obj", "sample_mat.json");
  transform->SetPosition({0, 0, 0});
  entity_id_ = entity.GetInstanceID();


  auto camera = Entity::Builder().Build();
  camera.AddComponent<Transform>();
  camera.AddComponent<Camera>();

  auto light_entity = Entity::Builder::Build();
  light_entity.AddComponent<Transform>()->SetPosition({0, 0, 2});
  light_entity.AddComponent<LightSource>()->SetColor({1, 0, 0, 1});
  light_[0] = light_entity.GetInstanceID();

  auto light_entity1 = Entity::Builder::Build();
  light_entity1.AddComponent<Transform>()->SetPosition({0, 0, -2});
  light_entity1.AddComponent<LightSource>()->SetColor({0, 0, 1, 1});
  light_[1] = light_entity1.GetInstanceID();

  auto sky_entity = Entity::Builder::Build();
  sky_entity.AddComponent<SkySource>("texture/default.jpg");

  FlushDriverCommand();
}


void VulkanRenderProcessor::AfterRender() {
  driver_->Commit(sch_);
  driver_->EndFrame(frame++);
  driver_->Tick();
  FlushDriverCommand();

  auto entity = ENTITY_CAST(entity_id_);
  auto transform = entity.GetComponent<Transform>();
  transform->SetRotate({frame, 2*frame, 3*frame});

  auto l1 = ENTITY_CAST(light_[0]);
  transform = l1.GetComponent<Transform>();
  transform->SetPosition({0, 3.f * glm::sin(frame/100.f),  3.f * glm::cos(frame/100.f)});

  auto l2 = ENTITY_CAST(light_[1]);
  transform = l2.GetComponent<Transform>();
  transform->SetPosition({-3.f * glm::sin(frame/100.f), 0,  -3.f * glm::cos(frame/100.f)});
}

void VulkanRenderProcessor::BeforeRender() {
  UpdateRenderContext();
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

void VulkanRenderProcessor::UpdateRenderContext() {
  RenderContext::WIDTH = DriverContext::Get().window_width_;
  RenderContext::HEIGHT = DriverContext::Get().window_height_;
  RenderContext::TIME_MS = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
  ).count();
}

}  // namespace our_graph