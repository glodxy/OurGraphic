//
// Created by 77205 on 2022/1/26.
//

#include "DeferredRenderer.h"
#include "include/GlobalEnum.h"
namespace our_graph {
using namespace our_graph::render_graph;

DeferredRenderer::DeferredRenderer(const SceneViewFamily *input, Driver *driver) : SceneRenderer(input, driver) {
  InitGBuffer();
}

void DeferredRenderer::InitGBuffer() {
  auto render_target_builder = RenderTarget::Builder(driver_);
  SamplerParams default_gbuffer_param;
  default_gbuffer_param.u = 0;
  Program::Sampler samplers[5];
  TargetBufferFlags target_flags = TargetBufferFlags::NONE;
  // todo:先使用5张纹理作为gbuffer
  for (uint8_t i = 0; i < 5; ++i) {
    Texture* tex = Texture::Builder(driver_)
        .Format(TextureFormat::RGBA8)
        .Sampler(SamplerType::SAMPLER_2D)
        .Usage(TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE)
        .Width(width_)
        .Height(height_)
        .Build();
    RenderTarget::AttachmentPoint point =
        static_cast<RenderTarget::AttachmentPoint>(i);
    render_target_builder.WithTexture(point, tex);
    target_flags |= GetTargetBufferFlagsAtColor(i);
  }
  gbuffer_.resource = render_target_builder.Build();
  gbuffer_.desc.samples = 1;
  gbuffer_.used_flags = target_flags;

  gbuffer_.desc.view_port.left = 0;
  gbuffer_.desc.view_port.bottom = 0;
  gbuffer_.desc.view_port.width = width_;
  gbuffer_.desc.view_port.height = height_;
}



void DeferredRenderer::Execute(const PerViewData &per_view, const std::vector<PerRenderableData> &renderables) {
  current_per_view_ = per_view;
  current_per_renderable_ = renderables;

  render_graph_ = new RenderGraph(allocator_);
  PrepareGeometryPass();
  PrepareLightPass();

  render_graph_->Compile();
  render_graph_->Execute(driver_);

  delete render_graph_;
}

}