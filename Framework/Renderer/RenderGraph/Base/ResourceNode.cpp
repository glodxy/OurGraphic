//
// Created by Glodxy on 2022/1/16.
//

#include "ResourceNode.h"
#include "Renderer/RenderGraph/RenderGraph.h"
#include "Renderer/RenderGraph/Base/PassNode.h"

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

bool ResourceNode::HasActiveWriters() const noexcept {
  DependencyGraph& dependency_graph = render_graph_.GetGraph();
  const auto& edges = dependency_graph.GetIncomingEdges(this);
  return !edges.empty();
}

ResourceEdgeBase * ResourceNode::GetReaderEdgeForPass(const PassNode *node) const noexcept {
  auto pos = std::find_if(reader_passes_.begin(), reader_passes_.end(), [node](const ResourceEdgeBase* edge) {
    return edge->to == node->GetID();
  });
  return pos != reader_passes_.end() ? *pos : nullptr;
}

ResourceEdgeBase * ResourceNode::GetWriteEdgeForPass(const PassNode *node) const noexcept {
  return write_pass_ && write_pass_->from == node->GetID() ? write_pass_ : nullptr;
}

bool ResourceNode::HasWriteFrom(const PassNode *node) const noexcept {
  return bool(GetWriteEdgeForPass(node));
}

void ResourceNode::SetParentReadDependency(ResourceNode *parent) noexcept {
  // 如果没有设置parent的read
  if (!parent_read_edge_) {
    parent_read_edge_ = new DependencyGraph::Edge(render_graph_.GetGraph(), parent, this);
  }
}

void ResourceNode::SetParentWriteDependency(ResourceNode *parent) noexcept {
  if (!parent_write_edge_) {
    parent_write_edge_ = new DependencyGraph::Edge(render_graph_.GetGraph(), this, parent);
  }
}

void ResourceNode::SetForwardResourceDependency(ResourceNode *source) noexcept {
  forwarded_edge_ = new DependencyGraph::Edge(render_graph_.GetGraph(), this, source);
}

void ResourceNode::ResolveResourceUsage(DependencyGraph &graph) noexcept {
  // 获取resource
  VirtualResource* resource = render_graph_.GetResource(resource_handle_);
  if (!resource) {
    LOG_ERROR("ResourceNode", "resolve resource_handle[{}] empty!", resource_handle_);
    assert(false);
    return;
  }
  // 如果有ref count，代表该resource有被使用，则resolve
  if (resource->ref_count_) {
    resource->ResolveUsage(graph, reader_passes_.data(), reader_passes_.size(), write_pass_);
  }
}

}  // namespace our_graph::render_graph