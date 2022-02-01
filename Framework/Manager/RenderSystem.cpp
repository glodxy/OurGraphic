//
// Created by Glodxy on 2021/11/23.
//

#include "RenderSystem.h"
#include "Utils/Event/APICaller.h"
#include "Framework/Resource/include/MeshReader.h"
#include "CameraSystem.h"
#include "Component/Transform.h"
#include "Resource/include/UniformStruct.h"
#include "Framework/include/GlobalEnum.h"
#include "Resource/include/MaterialInstance.h"
#include "Resource/include/Material.h"

// todo
#include "Resource/Material/ShaderCache.h"
#include "Resource/include/GlobalShaders.h"
#include "Renderer/DeferredRenderer.h"
namespace our_graph {
using utils::APICaller;

void RenderSystem::Init() {
  APICaller<RenderSystem>::RegisterAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID, weak_from_this());
  ShaderCache::Init();
  MeshReader::Init(driver_);
  GlobalShaders::Get().Init(driver_);
  renderer_ = std::make_shared<DeferredRenderer>(driver_);
  renderer_->Init();
//  // todo：设置参数（以后移除）
//  current_param_.clearColor = glm::vec4(0.f, 0.f, 1.f, 1.f);
//  current_param_.flags.clear = TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH;
//  current_param_.viewport.width = 0;
//  current_param_.viewport.height = 0;
//  current_param_.viewport.left = 0;
//  current_param_.viewport.bottom = 0;
//  current_param_.clearDepth = 1.0f;
//
//  //current_state_.shader_ = rh;
//  current_state_.raster_state_.colorWrite = true;
//  current_state_.raster_state_.depthWrite = true;
//  current_state_.raster_state_.depthFunc = SamplerCompareFunc::LE;
//  current_state_.raster_state_.culling = CullingMode::NONE;
//
//  per_view_uniform_ = std::make_unique<PerViewUniform>(driver_);
//
//  auto render_target_builder = RenderTarget::Builder(driver_);
//  SamplerParams default_gbuffer_param;
//  default_gbuffer_param.u = 0;
//  Program::Sampler samplers[5];
//  deferred_samplers_ = SamplerGroup(5);
//  // todo:先使用5张纹理作为gbuffer
//  for (uint8_t i = 0; i < 5; ++i) {
//    Texture* tex = Texture::Builder(driver_)
//        .Format(TextureFormat::RGBA8)
//        .Sampler(SamplerType::SAMPLER_2D)
//        .Usage(TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE)
//        .Width(800)
//        .Height(600)
//        .Build();
//    RenderTarget::AttachmentPoint point =
//        static_cast<RenderTarget::AttachmentPoint>(i);
//    render_target_builder.WithTexture(point, tex);
//    // todo: 暂时在此处完成deferred shader的绑定
//    samplers[i].name = std::string("gBuffer") + char('A' + i);
//    samplers[i].binding = i;
//    samplers[i].strict = true;
//
//    // todo:在此绑定
//    deferred_samplers_.SetSampler(i, tex->GetHandle(), default_gbuffer_param);
//  }
//  render_target_ = render_target_builder.Build();
//
//
//  Program program;
//  std::string vs_src = ShaderCache::GetDataFromFile("deferred_light.vs", ShaderVariantBit::DEFERRED_LIGHT);
//  std::string fs_src = ShaderCache::GetDataFromFile("deferred_light.fs", ShaderVariantBit::DEFERRED_LIGHT);
//  auto ds_vs = ShaderCache::CompileFile("deferred_vs",
//                                        shaderc_glsl_vertex_shader, vs_src);
//  auto ds_fs = ShaderCache::CompileFile("deferred_fs",
//                                        shaderc_glsl_vertex_shader, fs_src);
//  program.Diagnostics("deferred_light", ShaderVariantBit::DEFERRED_LIGHT)
//  .WithVertexShader(ds_vs.data(), ds_vs.size())
//  .WithFragmentShader(ds_fs.data(), ds_fs.size())
//  .SetSamplerGroup(BindingPoints::PER_VIEW, samplers, 5);
//
//  deferred_light_shader_ = driver_->CreateShader(std::move(program));
//  deferred_sampler_handle_ = driver_->CreateSamplerGroup(5);
//  driver_->UpdateSamplerGroup(deferred_sampler_handle_, deferred_samplers_.CopyAndClean());
}

void RenderSystem::Destroy() {
//  driver_->DestroyBufferObject(per_renderable_ubh_);
  renderer_->Destroy();
  APICaller<RenderSystem>::RemoveAPIHandler(SYSTEM_CALLER, SYSTEM_CALLER_ID);
}

std::string RenderSystem::GetSystemName() const {
  return "RenderSystem";
}

void RenderSystem::OnCameraUpdate() {

}

void RenderSystem::OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) {
  // todo：处理renderable
  // 生成相应的顶点
//  auto renderable = ComCast<Renderable>(com);
//  MeshReader reader(driver_);
//  reader.LoadMeshFromFile(renderable->GetMeshInfo().mesh_name);
  if (com->GetComponentType() == ComponentType::RENDERABLE) {
    renderables_.push_back(ComCast<Renderable>(com));
  }
}

void RenderSystem::Update(uint32_t time) {
  renderer_->Update(time);
}

void RenderSystem::Render() {
  auto camera = APICaller<CameraSystem>::CallAPI(SYSTEM_CALLER, SYSTEM_CALLER_ID,
                                                 &CameraSystem::GetMainCamera);
  SceneParams params;
  params.renderables = renderables_;
  params.cameras.push_back(camera);
  renderer_->Reset(&params);
  renderer_->Render();
}

}