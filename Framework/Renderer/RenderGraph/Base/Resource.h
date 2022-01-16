//
// Created by Glodxy on 2022/1/14.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RESOURCE_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RESOURCE_H_

#include "Renderer/RenderGraph/Base/DependencyGraph.h"
#include "Utils/OGLogging.h"

#include "Renderer/RenderGraph/Resource/RenderGraphTexture.h"
#include "Renderer/RenderGraph/Pass/RenderGraphRenderPass.h"

#include "Backend/include/Handle.h"
#include "Backend/include/DriverEnum.h"


namespace our_graph::render_graph {

class ResourceAllocatorInterface;

class PassNode;
class ResourceNode;
class ExternalRenderTarget;


class ResourceEdgeBase : public DependencyGraph::Edge {
 public:
  using DependencyGraph::Edge::Edge;
};

class VirtualResource {
 public:
  VirtualResource* parent_;
  const std::string name_;

  uint32_t ref_count_ = 0;
  PassNode* first_ = nullptr;
  PassNode* last_ = nullptr;

  explicit VirtualResource(const std::string& name) noexcept : parent_(this), name_(name) { }
  VirtualResource(VirtualResource* parent, const std::string& name) noexcept : parent_(parent), name_(name) {}
  VirtualResource(const VirtualResource& r) noexcept = delete;
  VirtualResource& operator=(const VirtualResource&) = delete;

  virtual ~VirtualResource() noexcept;

  // 更新first、last、refcount
  void NeededByPass(PassNode* node) noexcept;

  bool IsSubResource() const noexcept { return parent_ != this;}

  // 获取根节点的资源
  VirtualResource* GetResource() noexcept {
    VirtualResource* p =this;
    while (p->parent_ != p) {
      p = p->parent_;
    }
    return p;
  }

  // 在rendergraph的compile中调用
  virtual void ResolveUsage(DependencyGraph& graph,
                            const ResourceEdgeBase* const * edges,
                            size_t count,
                            const ResourceEdgeBase* writer) noexcept = 0;

  virtual void Devirtualize(ResourceAllocatorInterface& allocator) noexcept = 0;
  virtual void Destroy(ResourceAllocatorInterface& allocator) noexcept = 0;

  // 销毁由该resource所实例化的边
  virtual void DestroyEdge(DependencyGraph::Edge* edge) noexcept = 0;

  // 获取usage的字符串化
  virtual std::string UsageString() const noexcept = 0;

  virtual bool IsExternal() const noexcept {return false;}

  virtual ExternalRenderTarget* AsExternalRenderTarget() noexcept;

 protected:
  // 为该resource生成出边
  void AddOutgoingEdge(ResourceNode* node, ResourceEdgeBase* edge) noexcept;
  void SetIncomingEdge(ResourceNode* node, ResourceEdgeBase* edge) noexcept;

  // 以下转换避免了包含头文件
  static DependencyGraph::Node* ToDependencyGraphNode(ResourceNode* node) noexcept;
  static DependencyGraph::Node* ToDependencyGraphNode(PassNode* node) noexcept;
  static ResourceEdgeBase* GetReaderEdgeForPass(ResourceNode* resource_node, PassNode* pass_node) noexcept;
  static ResourceEdgeBase* GetWriterEdgeForPass(ResourceNode* resource_node, PassNode* pass_node) noexcept;
};


template<class RESOURCE>
class Resource : public VirtualResource {
  using Usage = typename RESOURCE::Usage;
  friend class RenderGraphResources;
 public:
  using Descriptor = typename RESOURCE::Descriptor;
  using SubResourceDescriptor = typename RESOURCE::SubResourceDescriptor;

  class ResourceEdge : public ResourceEdgeBase {
   public:
    Usage usage;
    ResourceEdge(DependencyGraph& graph,
                 DependencyGraph::Node* from, DependencyGraph::Node* to, Usage usage) noexcept
                 : ResourceEdgeBase(graph, from, to), usage(usage) {}
  };

  Resource(const std::string& name, const Descriptor& desc) noexcept
    : VirtualResource(name), descriptor_(desc){

  }

  Resource(Resource* parent, const std::string& name,
           const SubResourceDescriptor& desc) noexcept
           : VirtualResource(parent, name),
           descriptor_(RESOURCE::GenerateSubResourceDescriptor(parent->descriptor_, desc)),
           sub_resource_descriptor_(desc) {

  }

  ~Resource() noexcept override = default ;

  // 从pass到Resource
  virtual bool Connect(DependencyGraph& graph,
                       PassNode* pass_node, ResourceNode* resource_node, Usage u) {
    ResourceEdge* edge = static_cast<ResourceEdge*>(GetWriterEdgeForPass(resource_node, pass_node));
    if (edge) {
      edge->usage |= u;
    } else {
      edge = new ResourceEdge(graph,
                              ToDependencyGraphNode(pass_node),
                              ToDependencyGraphNode(resource_node),
                              u);
      SetIncomingEdge(resource_node, edge);
    }
    return true;
  }

  // 从Resource到Pass
  virtual bool Connect(DependencyGraph& graph,
                       ResourceNode* resource_node, PassNode* pass_node,
                       Usage u) {
    ResourceEdge* edge = static_cast<ResourceEdge*>(GetReaderEdgeForPass(resource_node, pass_node));
    if (edge) {
      edge->usage |= u;
    } else {
      edge = new ResourceEdge(graph,
                              ToDependencyGraphNode(resource_node), ToDependencyGraphNode(pass_node),
                              u);
      AddOutgoingEdge(resource_node, edge);
    }
    return true;
  }

 protected:
  virtual void ResolveUsage(DependencyGraph &graph,
                    const ResourceEdgeBase *const *edges,
                    size_t count, const ResourceEdgeBase *writer) noexcept override {
    for (size_t i = 0; i < count; ++i) {
      // 遍历所有边，找到所有可用边，设为使用
      if (graph.IsEdgeValid(edges[i])) {
        const ResourceEdge* const edge = static_cast<const ResourceEdge*>(edges[i]);
        usage_ |= edge->usage;
      }

      // 使用writer
      if (writer) {
        const ResourceEdge* const edge = static_cast<const ResourceEdge*>(writer);
        usage_ |= edge->usage;
      }

      // 更新parent的使用
      Resource* p = this;
      while (p != p->parent_) {
        p = static_cast<Resource*>(p->parent_);
        p->usage_ |= usage_;
      }
    }
  }

  virtual void DestroyEdge(DependencyGraph::Edge *edge) noexcept override {
    delete static_cast<ResourceEdge*>(edge);
  }

  virtual void Devirtualize(ResourceAllocatorInterface &allocator) noexcept override {
    // 非子资源，直接创建
    if (!IsSubResource()) {
      resource_.Create(allocator, name_, descriptor_, usage_);
    } else {
      // 子资源，直接取用parent的资源
      resource_ = static_cast<const Resource*>(parent_)->resource_;
    }
  }

  virtual void Destroy(ResourceAllocatorInterface& allocator) noexcept override {
    if (detached_ || IsSubResource()) {
      return;
    }
    resource_.Destroy(allocator);
  }

  virtual std::string UsageString() const noexcept override {
    return usage_.ToString();
  }



 protected:
  RESOURCE resource_{};
  Usage usage_{};

  // 这两者用于描述创建资源所需的信息
  Descriptor  descriptor_;
  SubResourceDescriptor sub_resource_descriptor_;

  // 该resource是否脱离了edge
  bool detached_ = false;
};

template<class RESOURCE>
class ExternalResource : public Resource<RESOURCE> {
 public:
  using Descriptor = typename RESOURCE::Descriptor;
  using Usage = typename RESOURCE::Usage;

  ExternalResource(const std::string& name,
                   const Descriptor& desc,
                   Usage usage,
                   const RESOURCE& resource) noexcept
                   : Resource<RESOURCE>(name, desc) {
    this->resource_ = resource;
    this->usage_ = usage;
  }

 protected:
  // 外部资源不需要此处来控制其销毁或生成
  void Devirtualize(ResourceAllocatorInterface &allocator) noexcept override {}
  void Destroy(ResourceAllocatorInterface& allocator) noexcept override {}

  bool IsExternal() const noexcept override {return true;}

  // 相较Resource的实现多了一个检查usage是否满足要求的过程
  bool Connect(DependencyGraph& graph,
               PassNode* pass_node, ResourceNode* resource_node, Usage u) override {
    if (!CheckConnext(u)) {
      return false;
    }
    return Resource<RESOURCE>::Connect(graph, resource_node, pass_node, u);
  }

 private:
  /**
   * @param u: RenderGraphTexture中的usage，
   * 即texture的使用，用于判断是否满足要求
   * */
  bool CheckConnext(Usage u) {
    if ((u & this->usage_) != u) {
      LOG_ERROR("Request usage {} not available on external resource[{}]"
                "with usage:{}",
                u.ToString(), this->name_, this->usage_.ToString());
      assert(false);
      return false;
    }
    return true;
  }
};

class ExternalRenderTarget : public ExternalResource<RenderGraphTexture> {
 public:
  ExternalRenderTarget(const std::string& name,
                       const RenderGraphTexture::Descriptor& main_attachment_desc,
                       const RenderGraphRenderPassInfo::ExternalDescriptor& external_desc,
                       RenderTargetHandle target);

  ~ExternalRenderTarget() noexcept override;
 protected:
  bool Connect(DependencyGraph& graph,
               PassNode* passNode, ResourceNode* resourceNode, RenderGraphTexture::Usage u) override;
  bool Connect(DependencyGraph& graph,
               ResourceNode* resourceNode, PassNode* passNode, RenderGraphTexture::Usage u) override;

  ExternalRenderTarget* AsExternalRenderTarget() noexcept override;

  RenderTargetHandle target_;
  RenderGraphRenderPassInfo::ExternalDescriptor external_desc_;

 private:
  bool CheckConnect(RenderGraphTexture::Usage u);

  static RenderGraphTexture::Usage GetUsageFromAttachmentsFlags(
      TargetBufferFlags attachments) noexcept;
};

}  // namespace our_graph::render_graph

class Resource {

};

#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RESOURCE_H_
