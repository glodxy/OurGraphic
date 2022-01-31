//
// Created by Home on 2022/1/30.
//

#include "SceneRenderer.h"
#include "Resource/Material/UniformBuffer.h"
#include "Resource/include/MeshReader.h"
#include "Resource/include/Material.h"
#include "include/GlobalEnum.h"
#include "Component/Renderable.h"
#include "Component/Transform.h"
#include "Component/Camera.h"
#include "Utils/Event/APICaller.h"
#include "Component/PerViewUniform.h"

namespace our_graph {
void Scene::Init(std::vector<std::shared_ptr<Renderable>> renderables) {
  material_instances_.clear();
  material_instances_.reserve(renderables.size());
  for (const auto& renderable : renderables) {
    material_instances_.push_back(renderable->GetMaterialInstance());
  }
}

void Scene::CommitAllMaterialInstance(Driver *driver) {
  for (auto mat : material_instances_) {
    mat->SetParameter("shadingModel", uint32_t (mat->GetMaterial()->GetShading()));
    mat->GetMaterial()->GetDefaultInstance()->Commit();
    mat->Commit();
  }
}

MaterialInstance *Scene::GetMaterialInstance(size_t idx) {
  return material_instances_[idx];
}

void ViewInfo::Init(Driver* driver, std::shared_ptr<Camera> camera) {
  camera_ = camera;
  viewport_ = camera->GetViewport();
  time_ = 0;
  if (per_view_uniform_) {
    delete per_view_uniform_;
  }
  per_view_uniform_ = new PerViewUniform(driver);
}
ViewInfo::~ViewInfo() {
  if (per_view_uniform_) {
    delete per_view_uniform_;
    per_view_uniform_ = nullptr;
  }
}

void ViewInfo::Update(uint32_t time) {
  viewport_ = camera_->GetViewport();
  time_ = time;
  per_view_uniform_->PrepareViewport(viewport_);
  per_view_uniform_->PrepareTime(time);
  per_view_uniform_->PrepareCamera(camera_);
}

PerViewUniform *ViewInfo::GetUniforms() {
  return per_view_uniform_;
}


void MeshCollector::Init(Driver* driver, std::vector<std::shared_ptr<Renderable>> renderables) {
  driver_ = driver;
  meshes_.clear();
  meshes_.resize(renderables.size());
  for (int i = 0; i < renderables.size(); ++i) {
    MeshReader::LoadMeshFromFile(renderables[i]->GetMeshInfo().mesh_name);
    meshes_[i].primitive = MeshReader::GetPrimitiveAt(0);
    meshes_[i].material_instance_idx = renderables[i]->GetMaterialInstance()->GetId();
    meshes_[i].uniforms.worldFromModelMat = utils::APICaller<Transform>::CallAPI("Component",  renderables[i]->GetEntity(),
                                                                                 &Transform::GetModelMatrix);
  }
}

void MeshCollector::CommitPerRenderableUniforms() {
  const size_t size = meshes_.size() * sizeof(PerRenderableUniformBlock);
  if (size > current_renderable_uniform_size_) {
    // 计算需要分配的renderable 个数，最少16个
    const size_t count = std::max(size_t(16u), (4u * meshes_.size() + 2u) / 3u);
    current_renderable_uniform_size_ = count * sizeof(PerRenderableUniformBlock);
    // 销毁原来的
    driver_->DestroyBufferObject(per_renderable_ubh_);
    per_renderable_ubh_ = driver_->CreateBufferObject(current_renderable_uniform_size_,
                                                      BufferObjectBinding::UNIFORM, BufferUsage::STREAM);
  }

  void* const buffer = driver_->Allocate(size);
  int idx = 0;
  for (auto& mesh : meshes_) {
    auto model_mat = mesh.uniforms.worldFromModelMat;
    const size_t offset = (idx++) * sizeof(PerRenderableUniformBlock);

    UniformBuffer::SetUniform(buffer, offset + UNIFORM_MEMBER_OFFSET(PerRenderableUniformBlock, worldFromModelMat),
                              model_mat);
  }
  driver_->UpdateBufferObject(per_renderable_ubh_, {buffer, size}, 0);
}

void MeshCollector::UsePerRenderableUniform(size_t idx) {
  driver_->BindUniformBufferRange(BindingPoints::PER_RENDERABLE, per_renderable_ubh_, sizeof(PerRenderableUniformBlock)*(idx), sizeof(PerRenderableUniformBlock));
}

size_t MeshCollector::GetSize() {
  return meshes_.size();
}

size_t MeshCollector::GetMaterialInstanceIdx(size_t idx) {
  return meshes_[idx].material_instance_idx;
}

RenderPrimitiveHandle MeshCollector::GetRenderPrimitive(size_t idx) {
  return meshes_[idx].primitive;
}


SceneRenderer::SceneRenderer(Driver *driver) : IRenderer(driver),
                                                allocator_(driver), render_graph_(allocator_) {

}

void SceneRenderer::Reset(void *params) {
  SceneParams* input = (SceneParams*) params;
  scene_->Init(input->renderables);
  mesh_collector_.Init(driver_, input->renderables);
  views_.resize(input->cameras.size());
  for (int i = 0; i < input->cameras.size(); ++i) {
    views_[i].Init(driver_, input->cameras[i]);
  }
}

void SceneRenderer::Update(uint32_t time) {
  for(auto& view : views_) {
    view.Update(time);
  }
  GC();
}

void SceneRenderer::GC() {
  allocator_.GC();
}



}  // namespace our_graph