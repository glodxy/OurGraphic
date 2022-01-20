//
// Created by Glodxy on 2022/1/13.
//

#include "PassNode.h"
#include "Renderer/RenderGraph/Base/ResourceNode.h"
namespace our_graph::render_graph {
PassNode::PassNode(RenderGraph &rg) noexcept
    : DependencyGraph::Node(rg.GetGraph()),
      render_graph_(rg) {

}

PassNode::PassNode(PassNode &&r) noexcept = default;
PassNode::~PassNode() noexcept = default;

std::string PassNode::GraphvizifyEdgeColor() const noexcept {
  return std::string("red");
}

void PassNode::RegisterResource(RenderGraphHandle resource_handle) noexcept {
  VirtualResource* resource = render_graph_.GetResource(resource_handle);
  resource->NeededByPass(this);
  declared_handles_.insert(resource_handle);
}

/*--------------RenderPassNode------------*/
RenderPassNode::RenderPassNode(RenderGraph &rg, const std::string &name, RenderGraphPassBase *base) noexcept
  : PassNode(rg), name_(name), pass_base_(base) {

}

RenderPassNode::RenderPassNode(RenderPassNode &&r) noexcept = default;
RenderPassNode::~RenderPassNode() noexcept = default;

void RenderPassNode::Execute(const RenderGraphResources &resources, Driver *driver) noexcept {
  RenderGraph& rg = render_graph_;
  ResourceAllocatorInterface& ra = rg.GetResourceAllocator();

  // devirtualize所有的render target
  for (auto& rt : render_target_data_) {
    rt.Devirtualize(rg, ra);
  }

  pass_base_->Execute(resources, driver);

  // destroy所有的render target
  for (auto& rt : render_target_data_) {
    rt.Destroy(ra);
  }
}

uint32_t RenderPassNode::DeclareRenderTarget(RenderGraph &rg,
                                             RenderGraph::Builder &builder,
                                             const std::string &name,
                                             const RenderGraphRenderPassInfo::Descriptor &desc) noexcept {
  RenderPassData data;
  data.name_ = name;
  data.descriptor_ = desc;
  RenderGraphRenderPassInfo::Attachments& attachments = data.descriptor_.attachments;


  const DependencyGraph& dependency_graph = rg.GetGraph();
  auto incoming_edges = dependency_graph.GetIncomingEdges(this);
  auto outgoing_edges = dependency_graph.GetOutgoingEdges(this);

  for (size_t i = 0; i < RenderPassData::ATTACHMENT_COUNT; ++ i) {
    // 未满足直接continue
    if (!desc.attachments.array[i]) {
      continue;
    }
    data.attachment_info_[i] = attachments.array[i];
    // 找到使用该render target的边(读取）
    auto incoming_pos = std::find_if(incoming_edges.begin(), incoming_edges.end(),
                                     [&dependency_graph, handle = desc.attachments.array[i]]
                                         (const DependencyGraph::Edge* edge) {
      const ResourceNode* node = static_cast<const ResourceNode*>(dependency_graph.GetNode(edge->from));
      return node->resource_handle_ == handle;
    });
    if (incoming_pos != incoming_edges.end()) {
      data.incoming_[i] = const_cast<ResourceNode*>(
          static_cast<const ResourceNode*>(dependency_graph.GetNode((*incoming_pos)->from));
    }

    // 检查outgoing(写）
    // 一个resource不能同时读写
    data.outgoing_[i] = rg.GetActiveResourceNode(desc.attachments.array[i]);
    if (data.outgoing_[i] == data.incoming_[i]) {
      data.outgoing_[i] = nullptr;
    }
  }

  uint32_t id = render_target_data_.size();
  render_target_data_.push_back(data);
  return id;
}
}  // namespace our_graph::render_graph