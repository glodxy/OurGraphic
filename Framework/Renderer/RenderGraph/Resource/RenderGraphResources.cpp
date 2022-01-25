//
// Created by Glodxy on 2022/1/15.
//

#include "RenderGraphResources.h"
#include "Renderer/RenderGraph/Base/PassNode.h"
#include "Utils/OGLogging.h"

namespace our_graph::render_graph {
RenderGraphResources::RenderGraphResources(RenderGraph &rg, PassNode &pass_node) noexcept
  : render_graph_(rg), pass_node_(pass_node) {

}

std::string RenderGraphResources::GetPassName() const noexcept {
  return pass_node_.GetName();
}

VirtualResource & RenderGraphResources::GetResource(RenderGraphHandle handle) const {
  if (!handle) {
    LOG_ERROR("RenderGraphResources", "handle invalid! pass[{}]",
              pass_node_.GetName());
    assert(false);
  }
  VirtualResource* const resource = render_graph_.GetResource(handle);

  auto& declared_handle = pass_node_.declared_handles_;
  // 判断该handle有没有被访问
  bool accessed = declared_handle.find(handle.GetIndex()) != declared_handle.end();

  if (!accessed) {
    LOG_ERROR("RenderGraphResources", "pass[{}] not access this resource[{}]",
              pass_node_.GetName(), resource->name_);
    assert(false);
  }

  if (!resource->ref_count_) {
    LOG_ERROR("RenderGraphResources", "resource[{}] no reference!",
              resource->name_);
    assert(false);
  }
  return *resource;
}

RenderGraphResources::RenderPassInfo RenderGraphResources::GetRenderPassInfo(uint32_t id) const {
  const RenderPassNode& render_pass_node = static_cast<const RenderPassNode&>(pass_node_);
  const RenderPassNode::RenderPassData* data = render_pass_node.GetRenderPassData(id);

  if (!data) {
    LOG_ERROR("RenderGraphResources", "GetRenderPassInfo Error! [{}] invalid",
               id);
    assert(false);
  }
  return {data->backend_.target_, data->backend_.params_};
}


}  // namespace our_graph::render_graph