//
// Created by Glodxy on 2022/1/3.
//

#include "PerViewUniform.h"
#include "Component/Camera.h"
#include "Utils/Event/APICaller.h"
#include "Component/Transform.h"
#include "Resource/include/SamplerStruct.h"
#include "Framework/include/GlobalEnum.h"
#include "Framework/Resource/include/SamplerStruct.h"
namespace our_graph {

PerViewUniform::PerViewUniform(Driver *driver) : driver_(driver) ,
                                                 per_view_sampler_(PerViewSamplerBlock::kSamplerCount) {
  per_view_ubh_ = driver_->CreateBufferObject(per_view_uniform_.GetSize(),
                                              BufferObjectBinding::UNIFORM,
                                              BufferUsage::DYNAMIC);

  per_view_sbh_ = driver_->CreateSamplerGroup(per_view_sampler_.GetSize());

  per_view_sampler_ = SamplerGroup(PerViewSamplerBlock::kSamplerCount);
}

void PerViewUniform::PrepareLight(uint32_t dynamic_count) {
  auto& s = per_view_uniform_.At(0);
  s.lightCount = dynamic_count;
}

void PerViewUniform::PrepareCamera(std::shared_ptr<Camera> camera_component) {
  auto view_from_world = camera_component->GetViewMatrix();
  auto clip_from_view = camera_component->GetProjMatrix();

  auto view_from_clip = glm::inverse(clip_from_view);
  auto world_from_view = glm::inverse(view_from_world);

  auto clip_from_world = clip_from_view * view_from_world;
  auto world_from_clip = glm::inverse(clip_from_world);

  auto& s = per_view_uniform_.At(0);
  s.viewFromWorldMat = view_from_world;
  s.worldFromViewMat = world_from_view;
  s.clipFromViewMat = clip_from_view;
  s.viewFromClipMat = view_from_clip;
  s.clipFromWorldMat = clip_from_world;
  s.worldFromClipMat = world_from_clip;

  s.cameraPosition = utils::APICaller<Transform>::CallAPI(CALL_COMPONENT,
                                                          camera_component->GetEntity(),
                                                          &Transform::GetPosition);
}

void PerViewUniform::PrepareViewport(math::Rect2D<float> rect) {
  float w = rect.r - rect.l;
  float h = rect.t - rect.b;
  auto& s = per_view_uniform_.At(0);
  s.resolution = math::Vec4(w, h, 1.f/w, 1.f/h);
}

void PerViewUniform::PrepareTime(uint32_t time) {
  auto& s = per_view_uniform_.At(0);
  s.time = time;
}


void PerViewUniform::Commit() {
  if (per_view_uniform_.IsDirty()) {
    driver_->UpdateBufferObject(per_view_ubh_, per_view_uniform_.ToBufferDescriptor(driver_), 0);
  }
  if (per_view_sampler_.IsDirty()) {
    driver_->UpdateSamplerGroup(per_view_sbh_, std::move(per_view_sampler_.CopyAndClean()));
  }
}

void PerViewUniform::Bind() {
  driver_->BindUniformBuffer(BindingPoints::PER_VIEW, per_view_ubh_);
  driver_->BindSamplers(BindingPoints::PER_VIEW, per_view_sbh_);
}

void PerViewUniform::Destroy() {
  driver_->DestroyBufferObject(per_view_ubh_);
  driver_->DestroySamplerGroup(per_view_sbh_);
}
}