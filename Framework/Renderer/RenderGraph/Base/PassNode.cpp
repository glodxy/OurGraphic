//
// Created by Glodxy on 2022/1/13.
//

#include "PassNode.h"
#include "Backend/include/DriverEnum.h"
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
    //! 因为render target实际存在于另外的resource node
    auto incoming_pos = std::find_if(incoming_edges.begin(), incoming_edges.end(),
                                     [&dependency_graph, handle = desc.attachments.array[i]]
                                         (const DependencyGraph::Edge* edge) {
      const ResourceNode* node = static_cast<const ResourceNode*>(dependency_graph.GetNode(edge->from));
      return node->resource_handle_ == handle;
    });
    if (incoming_pos != incoming_edges.end()) {
      data.incoming_[i] = const_cast<ResourceNode*>(
          static_cast<const ResourceNode*>(dependency_graph.GetNode((*incoming_pos)->from)));
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

void RenderPassNode::Resolve() noexcept {
  for (auto& rt : render_target_data_) {
    uint32_t min_width = std::numeric_limits<uint32_t>::max();
    uint32_t min_height = std::numeric_limits<uint32_t>::max();

    uint32_t max_width = 0;
    uint32_t max_height = 0;

    // 计算需要丢弃的flag
    ExternalRenderTarget* p_external_rt = nullptr;
    rt.backend_.params_.flags.discardStart = TargetBufferFlags::NONE;
    rt.backend_.params_.flags.discardEnd = TargetBufferFlags::NONE;

    for (size_t i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT + 2; ++i) {
      // 如果第i个attachment有设置
      if (rt.descriptor_.attachments.array[i]) {
        TargetBufferFlags target = GetTargetBufferFlagsAtColor(i);
        // 修改rt 对应的flag
        rt.target_buffer_flags_ |= target;

        // 如果该位置没有设置写入或者写入的resource node没有被其他节点使用
        if (!rt.outgoing_[i] || !rt.outgoing_[i]->HasActiveReaders()) {
          // 则代表该render target在使用后可以被丢弃
          rt.backend_.params_.flags.discardEnd |= target;
        }

        // 如果该位置没有写入或来源的resource node没有被其他pass写入
        if (!rt.incoming_[i] || !rt.incoming_[i]->HasWritePass()) {
          // 代表该rt可以在开始时被丢弃
          rt.backend_.params_.flags.discardStart |= target;
        }

        // 获取该render target的resource
        VirtualResource* p_resource = render_graph_.GetResource(rt.descriptor_.attachments.array[i]);
        Resource<RenderGraphTexture>* p_tex = static_cast<Resource<RenderGraphTexture>*>(p_resource);

        // 更新external rt找到第一个外部导入的render target resource
        p_external_rt = p_external_rt ? p_external_rt : p_tex->AsExternalRenderTarget();

        // 如果该rt sample=0，且该tex不会被sample，则同时更新该texture的信息
        if (!rt.descriptor_.samples &&
            none(p_tex->GetUsage() & TextureUsage::SAMPLEABLE)) {
          p_tex->GetDescriptor().samples = rt.descriptor_.samples;
        }

        // 此处更新获取所有attachment的min和max
        const uint32_t w = p_tex->GetDescriptor().width;
        const uint32_t h = p_tex->GetDescriptor().height;
        min_width = std::min(min_width, w);
        min_height = std::min(min_height, h);
        max_width = std::max(max_width, w);
        max_height = std::max(max_height, h);
      }

      // 设置clear所需要进行的discard(clear即开始时丢弃)
      rt.backend_.params_.flags.discardStart |= (rt.backend_.params_.flags.clear & rt.target_buffer_flags_);
    }
    // 如果没有设置任何的attachment,则直接跳过
    if (none(rt.target_buffer_flags_)) {
      LOG_ERROR("RenderPassNode", "Resolve RenderTarget[{}] error! not attachment set!",
                rt.name_);
      continue;
    }

    // 以所有attachment的最大w/h作为该rt的w/h
    uint32_t width = max_width;
    uint32_t height = max_height;

    // 更新viewport
    if (!rt.descriptor_.view_port.width) {
      rt.descriptor_.view_port.width = width;
    }
    if (!rt.descriptor_.view_port.height) {
      rt.descriptor_.view_port.height = height;
    }

    // todo：目前只处理第一个external
    // 如果存在外部导入的rt, 则直接用该外部rt来替换该rt
    if (p_external_rt) {
      rt.external_ = true;

      rt.target_buffer_flags_ = p_external_rt->GetExternalDesc().attachments;
      rt.descriptor_.view_port = p_external_rt->GetExternalDesc().view_port;
      rt.descriptor_.clear_color = p_external_rt->GetExternalDesc().clear_color;
      rt.descriptor_.clear_flags = p_external_rt->GetExternalDesc().clear_flags;
      rt.descriptor_.samples = p_external_rt->GetExternalDesc().samples;
      rt.backend_.target_ = p_external_rt->GetExternalTarget();

      // 不能丢弃外部rt的资源
      rt.backend_.params_.flags.discardStart &= ~p_external_rt->GetExternalDesc().keep_override_start;
      rt.backend_.params_.flags.discardEnd &= ~p_external_rt->GetExternalDesc().keep_override_end;
    }

    rt.backend_.params_.viewport = rt.descriptor_.view_port;
    rt.backend_.params_.clearColor = rt.descriptor_.clear_color;
    rt.backend_.params_.flags.clear = rt.descriptor_.clear_flags;
  }
}

void RenderPassNode::RenderPassData::Devirtualize(RenderGraph &rg,
                                                  ResourceAllocatorInterface &resource_allocator) noexcept {
  // 如果没有设置任何的attachment
  if (none(target_buffer_flags_)) {
    // 报错并返回
    LOG_ERROR("RenderPassData", "Devirtualize error! pass[{}] no attachments",
              name_);
    assert(false);
    return;
  }
  // 外部资源不需要devirtualize，直接return
  if (external_) {
    return;
  }

  MRT color_info {};
  // 设置color attachment
  for (size_t i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
    if (attachment_info_[i]) {
      const auto* p_tex = static_cast<Resource<RenderGraphTexture>*>(rg.GetResource(attachment_info_[i]));
      color_info[i].handle_ = p_tex->GetResource().handle;
      color_info[i].level_ = p_tex->GetSubDescriptor().level;
      color_info[i].layer_ = p_tex->GetSubDescriptor().layer;
    }
  }

  // 设置depth stencil
  TargetBufferInfo ds[2] = {};
  for (size_t i = 0; i < 2; ++i) {
    if (attachment_info_[MAX_SUPPORTED_RENDER_TARGET_COUNT + i]) {
      const auto* p_tex = static_cast<Resource<RenderGraphTexture>*>(rg.GetResource(attachment_info_[MAX_SUPPORTED_RENDER_TARGET_COUNT+i]));
      color_info[i].handle_ = p_tex->GetResource().handle;
      color_info[i].level_ = p_tex->GetSubDescriptor().level;
      color_info[i].layer_ = p_tex->GetSubDescriptor().layer;
    }
  }
  // 创建资源
  backend_.target_ = resource_allocator.CreateRenderTarget(
      name_, target_buffer_flags_,
      backend_.params_.viewport.width,
      backend_.params_.viewport.height,
      descriptor_.samples,
      color_info, ds[0], ds[1]);
}

void RenderPassNode::RenderPassData::Destroy(
    ResourceAllocatorInterface &resource_allocator) noexcept {
  if (!external_) {
    resource_allocator.DestroyRenderTarget(backend_.target_);
  }
}

const RenderPassNode::RenderPassData * RenderPassNode::GetRenderPassData(uint32_t id) const noexcept {
  return id < render_target_data_.size() ? &render_target_data_[id] : nullptr;
}
}  // namespace our_graph::render_graph