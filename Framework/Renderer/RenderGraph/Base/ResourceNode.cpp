//
// Created by Glodxy on 2022/1/16.
//

#include "ResourceNode.h"
#include "Renderer/RenderGraph/RenderGraph.h"

namespace our_graph::render_graph {

ResourceNode::ResourceNode(RenderGraph &rg, RenderGraphHandle h, RenderGraphHandle parent) noexcept
    : DependencyGraph::Node(rg.GetGraph()),
      resource_handle_(h), render_graph_(rg),
      parent_handle_(parent) {
}

ResourceNode::~ResourceNode() noexcept {
  // 1. 获取该节点所使用的资源
  VirtualResource* resource = render_graph_.GetResource(resource_handle_);
  // 2. 销毁该resource所属的边
  // 2.1 销毁写入
  resource->DestroyEdge(write_pass_);
  // 2.2 销毁读取边
  for (auto* edge : reader_passes_) {
    resource->DestroyEdge(edge);
  }

  //! 因为parent相关都是new出来的，所以需要销毁
  delete parent_read_edge_;
  delete parent_write_edge_;
  delete forwarded_edge_;
}

ResourceNode *ResourceNode::GetParentNode() noexcept {
  return parent_handle_.IsInitialized() ?
        render_graph_.GetActiveResourceNode(parent_handle_) : nullptr;
}

const char *ResourceNode::GetName() const noexcept {
  return render_graph_.GetResource(resource_handle_)->name_.c_str();
}

void ResourceNode::AddOutgoingEdge(ResourceEdgeBase *edge) noexcept {
  reader_passes_.push_back(edge);
}

void ResourceNode::SetIncomingEdge(ResourceEdgeBase *edge) noexcept {
  write_pass_ = edge;
}

bool ResourceNode::HasActiveReaders() const noexcept {
  DependencyGraph& dependency_graph = render_graph_.GetGraph();
  const auto& readers = dependency_graph.GetOutgoingEdges(this);

  for (const auto& reader : readers) {
    if (!dependency_graph.GetNode(reader->to)->IsCulled()) {
      return true;
    }
  }
  return false;
}


}  // namespace our_graph::render_graph