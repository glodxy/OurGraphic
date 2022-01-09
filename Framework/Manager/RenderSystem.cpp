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

// todo
#include "Resource/Material/ShaderCache.h"
namespace our_graph {
using utils::APICaller;

void RenderSystem::Init() {
  APICaller<RenderSystem>::RegisterAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID, weak_from_this());
  ShaderCache::Init();
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

  auto render_target_builder = RenderTarget::Builder(driver_);
  SamplerParams default_gbuffer_param;
  default_gbuffer_param.u = 0;
  Program::Sampler samplers[5];
  deferred_samplers_ = SamplerGroup(5);
  // todo:先使用5张纹理作为gbuffer
  for (uint8_t i = 0; i < 5; ++i) {
    Texture* tex = Texture::Builder(driver_)
        .Format(TextureFormat::RGBA8)
        .Sampler(SamplerType::SAMPLER_2D)
        .Usage(TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE)
        .Width(800)
        .Height(600)
        .Build();
    RenderTarget::AttachmentPoint point =
        static_cast<RenderTarget::AttachmentPoint>(i);
    render_target_builder.WithTexture(point, tex);
    // todo: 暂时在此处完成deferred shader的绑定
    samplers[i].name = std::string("gBuffer") + char('A' + i);
    samplers[i].binding = i;
    samplers[i].strict = true;

    // todo:在此绑定
    deferred_samplers_.SetSampler(i, tex->GetHandle(), default_gbuffer_param);
  }
  render_target_ = render_target_builder.Build();


  Program program;
  std::string vs_src = ShaderCache::GetDataFromFile("deferred_light.vs", ShaderVariantBit::DEFERRED_LIGHT);
  std::string fs_src = ShaderCache::GetDataFromFile("deferred_light.fs", ShaderVariantBit::DEFERRED_LIGHT);
  auto ds_vs = ShaderCache::CompileFile("deferred_vs",
                                        shaderc_glsl_vertex_shader, vs_src);
  auto ds_fs = ShaderCache::CompileFile("deferred_fs",
                                        shaderc_glsl_vertex_shader, fs_src);
  program.Diagnostics("deferred_light", ShaderVariantBit::DEFERRED_LIGHT)
  .WithVertexShader(ds_vs.data(), ds_vs.size())
  .WithFragmentShader(ds_fs.data(), ds_fs.size())
  .SetSamplerGroup(BindingPoints::PER_VIEW, samplers, 5);

  deferred_light_shader_ = driver_->CreateShader(std::move(program));
  deferred_sampler_handle_ = driver_->CreateSamplerGroup(5);
  driver_->UpdateSamplerGroup(deferred_sampler_handle_, deferred_samplers_.CopyAndClean());
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
    ins->SetParameter("shadingModel", uint32_t (ins->GetMaterial()->GetShading()));
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
  RenderPassParams geo_pass_param = current_param_;
  RenderPassParams param = current_param_;
  per_view_uniform_->Bind();
  // todo：目前先手动创建两个pass用作deferred light
  // 第一个pass
  driver_->BeginRenderPass(render_target_->GetHandle(), std::move(geo_pass_param));
  size_t idx = 0;
  for (auto& entity : components_) {
    auto& com_list = entity.second;
    auto com = GetComponentFromList(com_list, ComponentType::RENDERABLE);
    auto renderable = ComCast<Renderable>(com);
    auto mat = renderable->GetMaterialInstance();
    current_state_.raster_state_ = mat->GetMaterial()->GetRasterState();
    current_state_.shader_ = mat->GetMaterial()->GetShader(0);
    mat->Use();
    driver_->BindUniformBufferRange(BindingPoints::PER_RENDERABLE, per_renderable_ubh_, sizeof(PerRenderableUniformBlock)*(idx++), sizeof(PerRenderableUniformBlock));
    Render(renderable);
  }
  driver_->EndRenderPass();

  // 第二个pass
  // 使用单个screen space的pass
  // 先设置input
  SamplerGroup sampler_group;
  driver_->BeginRenderPass(rth, std::move(quad_param_));
  driver_->BindSamplers(0, deferred_sampler_handle_);
  driver_->Draw(quad_state_, MeshReader::GetQuadPrimitive());
  driver_->EndRenderPass();
  // todo:此处目前仅处理renderable
  // todo:目前所有的都走同一个pass以及pipeline state
  // todo:拆离Renderer以及RenderPass
//
//  for (auto& entity : components_) {
//    auto& com_list = entity.second;
//    auto com = GetComponentFromList(com_list, ComponentType::RENDERABLE);
//    auto renderable = ComCast<Renderable>(com);
//    auto mat = renderable->GetMaterialInstance();
//    current_state_.raster_state_ = mat->GetMaterial()->GetRasterState();
//    current_state_.shader_ = mat->GetMaterial()->GetShader();
//    mat->Use();
//    driver_->BindUniformBufferRange(BindingPoints::PER_RENDERABLE, per_renderable_ubh_, sizeof(PerRenderableUniformBlock)*(idx++), sizeof(PerRenderableUniformBlock));
//    Render(renderable);
//  }
//  driver_->EndRenderPass();
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