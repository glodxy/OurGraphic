//
// Created by 77205 on 2022/1/26.
//

#include "DeferredRenderer.h"
#include "include/GlobalEnum.h"
namespace our_graph {
using namespace our_graph::render_graph;

DeferredRenderer::DeferredRenderer(Driver *driver) : IRenderer(driver), allocator_(driver) {
  InitGBuffer();
}

void DeferredRenderer::InitGBuffer() {
  TargetBufferFlags target_flag = TargetBufferFlags::NONE;
  for (int i = 0; i < DeferredLightInputBinding::MAX; ++i) {
    target_flag |= GetTargetBufferFlagsAtColor(i);
  }
  gbuffer_.handle = driver_->CreateRenderTarget(target_flag, )
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