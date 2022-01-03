//
// Created by Glodxy on 2021/11/23.
//

#include "RenderSystem.h"
#include "Utils/Event/APICaller.h"
#include "MeshReader.h"
#include "CameraSystem.h"
#include "Component/Transform.h"
#include "Resource/include/UniformStruct.h"
#include "Framework/include/GlobalEnum.h"
#include "Resource/include/MaterialInstance.h"
#include "Resource/include/Material.h"
namespace our_graph {
using utils::APICaller;

void RenderSystem::Init() {
  APICaller<RenderSystem>::RegisterAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID, weak_from_this());

  // todo：设置参数（以后移除）
  current_param_.clearColor = glm::vec4(0.f, 0.f, 1.f, 1.f);
  current_param_.flags.clear = TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH;
  current_param_.viewport.width = 0;
  current_param_.viewport.height = 0;
  current_param_.viewport.left = 0;
  current_param_.viewport.bottom = 0;
  current_param_.clearDepth = 1.0f;

  //current_state_.shader_ = rh;
  current_state_.raster_state_.colorWrite = true;
  current_state_.raster_state_.depthWrite = true;
  current_state_.raster_state_.depthFunc = SamplerCompareFunc::LE;
  current_state_.raster_state_.culling = CullingMode::NONE;

  per_view_uniform_ = std::make_unique<PerViewUniform>(driver_);
}

void RenderSystem::Destroy() {
  per_view_uniform_->Destroy();
  driver_->DestroyBufferObject(per_renderable_ubh_);
  APICaller<RenderSystem>::RemoveAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID);
}

std::string RenderSystem::GetSystemName() const {
  return "RenderSystem";
}

void RenderSystem::OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) {
  // todo：处理renderable
  // 生成相应的顶点
  auto renderable = ComCast<Renderable>(com);
  MeshReader reader(driver_);
  reader.LoadMeshFromFile(renderable->GetMeshInfo().mesh_name);
}

void RenderSystem::PreparePerView() {
  // 1. 准备per view
  auto main_camera = APICaller<CameraSystem>::CallAPI(SYSTEM_CALLER, SYSTEM_CALLER_ID,
                                                      &CameraSystem::GetMainCamera);

  per_view_uniform_->PrepareCamera(main_camera);

  per_view_uniform_->Commit();
}

void RenderSystem::PrepareRenderable() {
  const size_t size = components_.size() * sizeof(PerRenderableUniformBlock);
  if (size > current_renderable_uniform_size_) {
    // 计算需要分配的renderable 个数，最少16个
    const size_t count = std::max(size_t(16u), (4u * components_.size() + 2u) / 3u);
    current_renderable_uniform_size_ = count * sizeof(PerRenderableUniformBlock);
    // 销毁原来的
    driver_->DestroyBufferObject(per_renderable_ubh_);
    per_renderable_ubh_ = driver_->CreateBufferObject(current_renderable_uniform_size_,
                                                      BufferObjectBinding::UNIFORM, BufferUsage::STREAM);
  }

  void* const buffer = driver_->Allocate(size);
  int idx = 0;
  for (auto& entity : components_) {
    auto model_mat = APICaller<Transform>::CallAPI("Component", entity.first,
                                                   &Transform::GetModelMatrix);
    const size_t offset = (idx++) * sizeof(PerRenderableUniformBlock);

    UniformBuffer::SetUniform(buffer, offset + UNIFORM_MEMBER_OFFSET(PerRenderableUniformBlock, worldFromModelMat),
                              model_mat);
  }
  driver_->UpdateBufferObject(per_renderable_ubh_, {buffer, size}, 0);
}

void RenderSystem::PrepareMaterial() {
  for (auto& entity : components_) {
    const auto& com_list = entity.second;
    auto renderable = ComCast<Renderable>(GetComponentFromList(com_list, RENDERABLE));
    MaterialInstance* ins = renderable->GetMaterialInstance();
    ins->Commit();
    ins->GetMaterial()->GetDefaultInstance()->Commit();
  }
}

void RenderSystem::Render() {
  // 准备数据
  PreparePerView();
  PrepareRenderable();
  PrepareMaterial();

  // todo:目前仅处理default的单render target
  // 从Camera获取rendertarget
  auto rth = APICaller<CameraSystem>::CallAPI(SYSTEM_CALLER, SYSTEM_CALLER_ID,
                                   &CameraSystem::GetRenderTarget,
                                   "default");
  RenderPassParams param = current_param_;

  per_view_uniform_->Bind();
  driver_->BeginRenderPass(rth, std::move(param));
  // todo:此处目前仅处理renderable
  // todo:目前所有的都走同一个pass以及pipeline state
  // todo:拆离Renderer以及RenderPass
  size_t idx = 0;
  for (auto& entity : components_) {
    auto& com_list = entity.second;
    auto com = GetComponentFromList(com_list, ComponentType::RENDERABLE);
    auto renderable = ComCast<Renderable>(com);
    auto mat = renderable->GetMaterialInstance();
    current_state_.raster_state_ = mat->GetMaterial()->GetRasterState();
    current_state_.shader_ = mat->GetMaterial()->GetShader();
    mat->Use();
    driver_->BindUniformBufferRange(BindingPoints::PER_RENDERABLE, per_renderable_ubh_, sizeof(PerRenderableUniformBlock)*(idx++), sizeof(PerRenderableUniformBlock));
    Render(renderable);
  }
  driver_->EndRenderPass();
}

void RenderSystem::PrepareRender(std::shared_ptr<Renderable> renderable) {
  MeshReader reader(driver_);
  reader.LoadMeshFromFile(renderable->GetMeshInfo().mesh_name);
}

void RenderSystem::Render(std::shared_ptr<Renderable> renderable) {
  auto mesh_info = renderable->GetMeshInfo();
  MeshReader reader(driver_);
  reader.LoadMeshFromFile(renderable->GetMeshInfo().mesh_name);
  uint32_t size = reader.GetMeshSize();
  for (int i = 0; i < size; ++i) {
    LOG_INFO("RenderSystem", "DrawCall!");
    driver_->Draw(current_state_, reader.GetPrimitiveAt(i));
  }
}

}