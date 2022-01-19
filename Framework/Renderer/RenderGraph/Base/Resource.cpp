//
// Created by Glodxy on 2022/1/14.
//

#include "Resource.h"

#include "Renderer/RenderGraph/Base/DependencyGraph.h"
#include "Renderer/RenderGraph/Base/ResourceNode.h"
#include "Renderer/RenderGraph/Base/PassNode.h"

namespace our_graph::render_graph {

VirtualResource::~VirtualResource() noexcept  = default;

void VirtualResource::AddOutgoingEdge(ResourceNode *node, ResourceEdgeBase *edge) noexcept {
  node->AddOutgoingEdge(edge);
}

void VirtualResource::SetIncomingEdge(ResourceNode *node, ResourceEdgeBase *edge) noexcept {
  node->SetIncomingEdge(edge);
}

DependencyGraph::Node *VirtualResource::ToDependencyGraphNode(PassNode *node) noexcept {
  return node;
}

DependencyGraph::Node *VirtualResource::ToDependencyGraphNode(ResourceNode *node) noexcept {
  return node;
}

ResourceEdgeBase *VirtualResource::GetReaderEdgeForPass(
    ResourceNode *resource_node, PassNode *pass_node) noexcept {
  return resource_node->GetReaderEdgeForPass(pass_node);
}

ResourceEdgeBase *VirtualResource::GetWriteEdgeForPass(
    ResourceNode *resource_node, PassNode *pass_node) noexcept {
  return resource_node->GetWriteEdgeForPass(pass_node);
}

/**
 * 该函数说明该resource被一个pass使用
 * 故需要更新相关信息
 * */
void VirtualResource::NeededByPass(PassNode *node) noexcept {
  ref_count_++;

  // 因为外部赋予的pass node是按照顺序调用的
  // 即该函数的node参数会符合顺序，故无需判断，只需要检查当前是否有值
  first_ = first_ ? first_ : node;
  last_ = node;

  if (parent_ != this) {
    parent_->NeededByPass(node);
  }
}

/*----------------------------------External RenderTarget---------------------*/
ExternalRenderTarget::~ExternalRenderTarget() noexcept = default;

ExternalRenderTarget::ExternalRenderTarget(const std::string &name,
                                           const RenderGraphTexture::Descriptor &main_attachment_desc,
                                           const RenderGraphRenderPassInfo::ExternalDescriptor &external_desc,
                                           RenderTargetHandle target)
                                           : ExternalResource<RenderGraphTexture>(name, main_attachment_desc,
                                                                                  GetUsageFromAttachmentsFlags(external_desc.attachments), {}),
                                             target_(target), external_desc_(external_desc) {
}

//! 作为rendertarget的texture只能允许作为attachment
bool ExternalRenderTarget::CheckConnect(RenderGraphTexture::Usage u) {
  constexpr auto FLAGS = RenderGraphTexture::Usage::COLOR_ATTACHMENT |
                          RenderGraphTexture::Usage::DEPTH_ATTACHMENT |
                          RenderGraphTexture::Usage::STENCIL_ATTACHMENT;

  if (any(u & (~FLAGS))) {
    return false;
  }
  return true;
}

bool ExternalRenderTarget::Connect(DependencyGraph &graph,
                                   PassNode *passNode,
                                   ResourceNode *resourceNode,
                                   RenderGraphTexture::Usage u) {
  if (!CheckConnect(u)) {
    return false;
  }
  return Resource::Connect(graph, passNode, resourceNode, u);
}

bool ExternalRenderTarget::Connect(DependencyGraph &graph,
                                   ResourceNode *resourceNode,
                                   PassNode *passNode,
                                   RenderGraphTexture::Usage u) {
  if (!CheckConnect(u)) {
    return false;
  }
  return Resource::Connect(graph, resourceNode, passNode, u);
}

RenderGraphTexture::Usage ExternalRenderTarget::GetUsageFromAttachmentsFlags(TargetBufferFlags attachments) noexcept {
  if (any(attachments & TargetBufferFlags::COLOR_ALL)) {
    return RenderGraphTexture::Usage::COLOR_ATTACHMENT;
  }

  if ((attachments & TargetBufferFlags::DEPTH_AND_STENCIL) == TargetBufferFlags::DEPTH_AND_STENCIL) {
    return RenderGraphTexture::Usage::DEPTH_ATTACHMENT | RenderGraphTexture::Usage::STENCIL_ATTACHMENT;
  }

  if (any(attachments & TargetBufferFlags::DEPTH)) {
    return RenderGraphTexture::Usage::DEPTH_ATTACHMENT;
  }

  if (any(attachments & TargetBufferFlags::STENCIL)) {
    return RenderGraphTexture::Usage::STENCIL_ATTACHMENT;
  }

  return RenderGraphTexture::Usage::COLOR_ATTACHMENT;
}


template class Resource<RenderGraphTexture>;
template class ExternalResource<RenderGraphTexture>;
}  // namespace our_graph::render_graph