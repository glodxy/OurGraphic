//
// Created by Glodxy on 2022/1/16.
//

#include "RenderGraph.h"
#include "Renderer/RenderGraph/Base/PassNode.h"
#include "Renderer/RenderGraph/Pass/RenderGraphPass.h"
namespace our_graph::render_graph {

inline RenderGraph::Builder::Builder(RenderGraph &rg, PassNode *node) noexcept
  : render_graph_(rg), pass_node_(node) {
}

void RenderGraph::Builder::SideEffect() noexcept {
  pass_node_->MakeTarget();
}

//! 获取handle的名字
std::string RenderGraph::Builder::GetName(RenderGraphHandle handle) const {
  return render_graph_.GetResource(handle)->name_;
}

uint32_t RenderGraph::Builder::DeclareRenderPass(
    const std::string &name,
    const RenderGraphRenderPassInfo::Descriptor &desc) {
  return static_cast<RenderPassNode*>(pass_node_)
    ->DeclareRenderTarget(render_graph_, *this, name, desc);
}

RenderGraphId<RenderGraphTexture> RenderGraph::Builder::DeclareRenderPass(
    RenderGraphId<RenderGraphTexture> color,
    uint32_t *idx) {
  //! 将该color标记为write
  color = Write(color);
  uint32_t id = DeclareRenderPass(GetName(color),
                                  {.attachments = { .color = {color}}});
  if (idx) *idx = id;
  return color;
}

RenderGraph::RenderGraph(ResourceAllocatorInterface &allocator_interface)
  : resource_allocator_(allocator_interface) {
  resource_slots_.reserve(256);
  resources_.reserve(256);
  resource_nodes_.reserve(256);
  pass_nodes_.reserve(64);
}

void RenderGraph::DestroyInternal() {
  //! 销毁所有分配的内存
  allocator_.DestroyAll();
}

RenderGraph::~RenderGraph() noexcept {
  DestroyInternal();
}

void RenderGraph::Reset() noexcept {
  DestroyInternal();
  pass_nodes_.clear();
  resource_nodes_.clear();
  resources_.clear();
  resource_slots_.clear();
}

RenderGraph &RenderGraph::Compile() noexcept {
  DependencyGraph& dependency_graph = graph_;
  //1. 首先裁剪一些无用的节点
  dependency_graph.Cull();

  //2. 进行拆分，将其分为active、非active两部分
  active_pass_end_ = std::stable_partition(
      pass_nodes_.begin(), pass_nodes_.end(), [](const PassNode* node) {
        return !node->IsCulled();
      });

  auto first = pass_nodes_.begin();
  const auto active_pass_node_end = active_pass_end_;
  while (first != active_pass_node_end) {
    const PassNode* pass_node = *first;
  }
}



}  // namespace our_graph::render_graph