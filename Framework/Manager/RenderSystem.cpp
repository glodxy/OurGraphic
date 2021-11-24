//
// Created by Glodxy on 2021/11/23.
//

#include "RenderSystem.h"
#include "Utils/Event/APICaller.h"
#include "MeshReader.h"
#include "CameraSystem.h"
#include "Framework/Resource/ShaderCache.h"
namespace our_graph {
using utils::APICaller;
// todo: 换位置
struct PerFrameUniform {
  math::Mat4 world_to_view;
  math::Mat4 view_to_clip;
};

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

  ShaderCache::ShaderBuilder builder = ShaderCache::ShaderBuilder("test_shader", &shader_cache_);
  Program program = builder.Vertex("common_mesh.vert").Frag("common_mesh.frag").Build();
  auto rh = driver_->CreateShader(std::move(program));
  current_state_.shader_ = rh;
  current_state_.raster_state_.colorWrite = true;
  current_state_.raster_state_.culling = CullingMode::NONE;

  per_frame_uniform_ = driver_->CreateBufferObject(sizeof(PerFrameUniform), BufferObjectBinding::UNIFORM, BufferUsage::STREAM);

}

void RenderSystem::Destroy() {
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

void RenderSystem::Render() {
  // 准备数据
  // todo:camera与RenderPass绑定，而非在此处设置
  PerFrameUniform uniform;
  auto main_camera = APICaller<CameraSystem>::CallAPI(SYSTEM_CALLER, SYSTEM_CALLER_ID,
                                                      &CameraSystem::GetMainCamera);
  math::Mat4 tmp(1);
  tmp[3][0] = 0.5f;
  uniform.world_to_view = main_camera->GetViewMatrix();
  uniform.view_to_clip = main_camera->GetProjMatrix();
  void* uniform_data = ::malloc(sizeof(PerFrameUniform));
  memcpy(uniform_data, &uniform, sizeof(PerFrameUniform));
  driver_->UpdateBufferObject(per_frame_uniform_, BufferDescriptor(uniform_data, sizeof(PerFrameUniform), [](void* buffer, size_t size, void* user) {
    ::free(buffer);
  }), 0);
  driver_->BindUniformBuffer(1, per_frame_uniform_);


  // todo:目前仅处理default的单render target
  // 从Camera获取rendertarget
  auto rth = APICaller<CameraSystem>::CallAPI(SYSTEM_CALLER, SYSTEM_CALLER_ID,
                                   &CameraSystem::GetRenderTarget,
                                   "default");
  RenderPassParams param = current_param_;
  driver_->BeginRenderPass(rth, std::move(param));
  // todo:此处目前仅处理renderable
  // todo:目前所有的都走同一个pass以及pipeline state
  // todo:拆离Renderer以及RenderPass
  for (auto& entity : components_) {
    auto& com_list = entity.second;
    auto com = GetComponentFromList(com_list, ComponentType::RENDERABLE);
    auto renderable = ComCast<Renderable>(com);
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