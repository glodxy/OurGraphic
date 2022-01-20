//
// Created by Glodxy on 2022/1/16.
//

#include "RenderGraph.h"
#include "Renderer/RenderGraph/Base/PassNode.h"
#include "Renderer/RenderGraph/Pass/RenderGraphPass.h"
#include "Renderer/RenderGraph/Base/ResourceNode.h"
#include "Utils/Memory/SImpleAllocator.h"
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
  auto& allocator = allocator_;
  // 1. 销毁所有pass
  std::for_each(pass_nodes_.begin(), pass_nodes_.end(), [&allocator](auto item) {
    allocator.Destroy(item);
  });
  // 2. 销毁resource node
  std::for_each(resource_nodes_.begin(), resource_nodes_.end(), [&allocator](auto item) {
    allocator.Destroy(item);
  });
  // 3. 销毁resource
  std::for_each(resources_.begin(), resources_.end(), [&allocator](auto item) {
    allocator.Destroy(item);
  });
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
  // 3. 处理所有的active node
  while (first != active_pass_node_end) {
    PassNode* const pass_node = *first;
    first++;
    //! 如果该node已被裁剪，则代表划分出了问题，报错
    if (pass_node->IsCulled()) {
      LOG_ERROR("RenderGraphCompile", "{} node culled!", pass_node->GetName());
      assert(false);
    }

    // 3.1 获取该node的所有入边（即所有读取的资源)
    const auto& reads = graph_.GetIncomingEdges(pass_node);
    for (const auto& edge : reads) {
      //! 需检查edge是否可用
      if (!graph_.IsEdgeValid(edge)) {
        LOG_ERROR("RenderGraphCompile", "edge invalid!");
        assert(false);
      }
      // 获取该edge的from，即读取的资源节点
      auto p_node = static_cast<ResourceNode*>(graph_.GetNode(edge->from));
      // 将该节点注册至pass
      pass_node->RegisterResource(p_node->resource_handle_);
    }

    // 3.2 处理所有出边（即所有写入的资源)
    const auto& writes = graph_.GetOutgoingEdges(pass_node);
    for (const auto& edge : writes) {
      auto p_node = static_cast<ResourceNode*>(graph_.GetNode(edge->to));
      pass_node->RegisterResource(p_node->resource_handle_);
    }

    // 3.3 解析该node
    pass_node->Resolve();
  }

  // 4. 处理resource node，设置对应的需要devirtualize以及destroy的资源
  for (auto* p_resource : resources_) {
    VirtualResource* resource = p_resource;
    // 如果存在引用，代表该资源需要加入node管理
    if (resource->ref_count_) {
      PassNode* p_first = resource->first_;
      PassNode* p_last = resource->last_;
      //! 只有都存在时才代表其被正常使用
      if (p_first && p_last) {
        if (p_first->IsCulled() || p_last->IsCulled()) {
          LOG_ERROR("RenderGraphCompile", "resource[{}]'s pass culled!", resource->name_);
          assert(false);
          continue;
        }
        p_first->devirtualize_.push_back(resource);
        p_last->destroy_.push_back(resource);
      }
    }
  }

  // todo：需要维护active的resource
  // 5. 解析resource的使用
  for (auto* resource_node : resource_nodes_) {
    resource_node->ResolveResourceUsage(graph_);
  }
  return *this;
}

void RenderGraph::Execute(Driver *driver) noexcept {
  const auto& pass_nodes = pass_nodes_;
  auto& resource_allocator = resource_allocator_;

  auto first = pass_nodes.begin();
  const auto active_pass_end = active_pass_end_;

  while (first != active_pass_end) {
    PassNode* const node = *first;
    first ++;
    if (node->IsCulled()) {
      LOG_ERROR("RenderGraphExecute", "pass[{}] culled!", node->GetName());
      assert(false);
      continue;
    }

    // 首先devirtualize所有需要在此实例化的资源
    for (VirtualResource* v_src : node->devirtualize_) {
      //! 此处加个保护，确保该resource是在该node实例化
      if (v_src->first_ != node) {
        LOG_ERROR("RenderGraphExecute", "resource[{}] should not devirtualize here!", v_src->name_);
        assert(false);
        continue;
      }
      v_src->Devirtualize(resource_allocator);
    }

    // 构建资源包
    RenderGraphResources resources(*this, *node);
    // 执行
    node->Execute(resources, driver);

    // 销毁需要销毁的资源
    for (VirtualResource* v_src : node->destroy_) {
      //! 保护，确保该resouce在node销毁
      if (v_src->last_ != node) {
        LOG_ERROR("RenderGraphExecute", "resource[{}] should not destroy here!", v_src->name_);
        assert(false);
        continue;
      }
      v_src->Destroy(resource_allocator);
    }
  }

  // 刷新driver
  driver->Flush();
}

void RenderGraph::AddPresentPass(std::function<void(Builder & )> setup) noexcept {
  PresentPassNode* present_pass_node = (PresentPassNode*)allocator_.Make<PresentPassNode>(*this);
  pass_nodes_.push_back(present_pass_node);
  Builder builder(*this, present_pass_node);
  setup(builder);
  builder.SideEffect();
}

RenderGraph::Builder RenderGraph::AddPassInternal(const std::string &name, RenderGraphPassBase *base) noexcept {
  //1. 创建pass node
  PassNode* node = allocator_.Make<RenderPassNode>(*this, name, base);
  base->SetNode(node);
  pass_nodes_.push_back(node);
  return Builder(*this, node);
}

RenderGraphHandle RenderGraph::CreateNewVersion(RenderGraphHandle handle, RenderGraphHandle parent) noexcept {
  ResourceSlot& slot = GetResourceSlot(handle);
  slot.version = ++handle.version_;
  slot.nid = resource_nodes_.size();
  ResourceNode* new_node = allocator_.Make<ResourceNode>(*this, handle, parent);
  resource_nodes_.push_back(new_node);
  return handle;
}

RenderGraphHandle RenderGraph::CreateNewVersionForSubresourceIfNeeded(RenderGraphHandle handle) noexcept {
  ResourceSlot& slot = GetResourceSlot(handle);
  // 代表目前没有sub resource
  if (slot.sid < 0) {
    slot.sid = slot.nid;
    slot.nid = resource_nodes_.size();
    ResourceNode* parent = allocator_.Make<ResourceNode>(*this, handle, RenderGraphHandle{});
    resource_nodes_.push_back(parent);
  }
  return handle;
}

RenderGraphHandle RenderGraph::AddResourceInternal(VirtualResource *resource) noexcept {
  return AddSubResourceInternal(RenderGraphHandle{}, resource);
}

RenderGraphHandle RenderGraph::AddSubResourceInternal(
    RenderGraphHandle parent, VirtualResource *sub) noexcept {
  // 生成新的handle
  RenderGraphHandle handle(resource_slots_.size());
  ResourceSlot& slot = resource_slots_.emplace_back();
  slot.rid = resources_.size();
  slot.nid = resource_nodes_.size();
  resources_.push_back(sub);
  ResourceNode* node = allocator_.Make<ResourceNode>(*this, handle, parent);
  resource_nodes_.push_back(node);
  return handle;
}

RenderGraphHandle RenderGraph::ReadInternal(RenderGraphHandle handle,
                                            PassNode *pass,
                                            std::function<bool(ResourceNode *, VirtualResource *)> connect) {
  // handle不可用时，直接返回空
  if (!IsValid(handle)) {
    return {};
  }

  VirtualResource* const resource = GetResource(handle);
  ResourceNode* const resource_node = GetActiveResourceNode(handle);

  // 检查该pass是否有对该resource进行写操作，
  // 写与读只允许存在一个
  bool has_write = resource_node->HasWriteFrom(pass);
  if (has_write) {
    LOG_ERROR("RenderGraphReadInternal", "resource[{}] has been writen by pass[{}]",
              resource_node->GetName(), pass->GetName());
    return {};
  }

  if (!resource_node->HasWritePass() && !resource->IsExternal()) {
    //! 如果该resource没有写数据，且非外部资源（在外部写入数据）
    //! 代表该resource是空读状态
    LOG_ERROR("RenderGraphReadInternal", "resource[{}] no data write!",
              resource_node->GetName());
    return {};
  }

  // 尝试连接pass与resource
  if (!connect(resource_node, resource)) {
    LOG_ERROR("RenderGraphReadInternal", "resource[{}] connect failed!",
              resource_node->GetName());
    return {};
  }
  if (resource->IsSubResource()) {
    // 当该resource为sub resource时，需要对其parent也进行read
    auto* parent_node = resource_node->GetParentNode();
    ResourceSlot& parent_slot = GetResourceSlot(parent_node->resource_handle_);
    // 确认该parent slot确实有sub resource
    // 使用该sub resource作为parent node的副本来进行read
    if (parent_slot.sid >= 0) {
      parent_node = resource_nodes_[parent_slot.sid];
    }
    resource_node->SetParentReadDependency(parent_node);
  }

  ResourceSlot& slot = GetResourceSlot(handle);
  if (slot.sid >= 0) {
    slot.sid = -1;
  }
  return handle;
}


RenderGraphHandle RenderGraph::WriteInternal(RenderGraphHandle handle,
                                             PassNode *pass,
                                             std::function<bool(ResourceNode *, VirtualResource *)> connect) {
  if (!IsValid(handle)) {
    return {};
  }

  VirtualResource* const resource = GetResource(handle);
  ResourceNode* node = GetActiveResourceNode(handle);
  ResourceNode* parent_node = node->GetParentNode();

  // 是sub resource时，则创建一个parent node
  if (resource->IsSubResource()) {
    CreateNewVersionForSubresourceIfNeeded(parent_node->resource_handle_);
    parent_node = node->GetParentNode();
  }

  // 没有从该pass写入时
  if (!node->HasWriteFrom(pass)) {
    // 没有写入，也没有读取时，代表这是一个新的节点，不需要额外创建来防止多写入
    if (!node->HasWritePass() && !node->HasReaders()) {

    } else {
      // 创建一个副本
      handle = CreateNewVersion(handle, parent_node ? parent_node->resource_handle_ : RenderGraphHandle{});
      node = GetActiveResourceNode(handle);
    }
  }

  if (!connect(node, resource)) {
    // 连接失败，需要重置所有操作
    // todo
    return {};
  }

  if (resource->IsSubResource()) {
    node->SetParentWriteDependency(parent_node);
  }
  if (resource->IsExternal()) {
    node->MakeTarget();
  }
  return handle;
}

RenderGraphHandle RenderGraph::ForwardResourceInternal(RenderGraphHandle handle, RenderGraphHandle replaced_handle) {
  if (!IsValid(handle)) {
    return {};
  }
  if (!IsValid(replaced_handle)) {
    return {};
  }

  // 设置依赖
  GetActiveResourceNode(replaced_handle)->SetForwardResourceDependency(GetActiveResourceNode(handle));

  const ResourceSlot& src_slot = GetResourceSlot(handle);
  ResourceSlot& replaced_slot = GetResourceSlot(replaced_handle);

  replaced_slot.rid = src_slot.rid;
  // todo:sid

  replaced_slot.version = -1;
  return handle;
}

RenderGraphId<RenderGraphTexture> RenderGraph::Import(const std::string &name,
                                                      const RenderGraphRenderPassInfo::ExternalDescriptor &desc,
                                                      RenderTargetHandle render_target) {
  VirtualResource* resource = allocator_.Make<ExternalRenderTarget>(name,
                                                                    RenderGraphTexture::Descriptor{
    .width = desc.view_port.width,
    .height = desc.view_port.height
  }, desc, render_target);
  return RenderGraphId<RenderGraphTexture>(AddResourceInternal(resource));
}

bool RenderGraph::IsValid(RenderGraphHandle handle) const {
  if (!handle.IsInitialized()) {
    return false;
  }
  ResourceSlot slot = GetResourceSlot(handle);
  if (slot.version != handle.version_) {
    LOG_ERROR("RenderGraphHandleCheck", "handle[{}] version error!"
                                        "slot:{}, handle:{}",
                                        handle.index_,
                                        slot.version,
                                        handle.version_);
    return false;
  }
  return true;
}

bool RenderGraph::IsCulled(const RenderGraphPassBase &pass) const noexcept {
  return pass.GetNode()->IsCulled();
}

bool RenderGraph::IsAcyclic() const noexcept {
  return graph_.IsAcyclic();
}

void RenderGraph::ExportGraphviz(std::ostream &out, const std::string &name) {
  graph_.ExportGraphviz(out, name);
}

}  // namespace our_graph::render_graph