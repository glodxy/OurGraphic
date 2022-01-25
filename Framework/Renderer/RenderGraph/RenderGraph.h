//
// Created by Glodxy on 2022/1/16.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPH_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPH_H_
#include <cstdint>
#include <string>
#include <functional>
#include <memory>

#include "Renderer/RenderGraph/Pass/RenderGraphRenderPass.h"
#include "Renderer/RenderGraph/Base/RenderGraphId.h"
#include "Renderer/RenderGraph/Base/Blackboard.h"
#include "Renderer/RenderGraph/Base/DependencyGraph.h"
#include "Renderer/RenderGraph/Pass/RenderGraphPass.h"
#include "Renderer/RenderGraph/Base/IResourceAllocator.h"

#include "Utils/Memory/SImpleAllocator.h"

namespace our_graph::render_graph {


class RenderGraphPassExecutor;
class PassNode;
class ResourceNode;
class VirtualResource;

// todo
class RenderGraph {
 public:
  class Builder {
   public:
    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;

    /**
     * 生成一个在当前pass中使用的render pass
     * @param name:pass的名称
     * @param desc:所需要的descriptor
     * @return 该render pass的idx(一个renderpass node含有多个render pass)
     * */
    uint32_t DeclareRenderPass(const std::string& name,
                               const RenderGraphRenderPassInfo::Descriptor& desc);


    /**
     * 快速生成一个color的attachment的render pass
     * @param color：所需要绑定的attachment
     * @param color：返回attachment的idx
     * @return 返回设置后的sub resource的新handle
     * */
    RenderGraphId<RenderGraphTexture> DeclareRenderPass(
        RenderGraphId<RenderGraphTexture> color, uint32_t* idx = nullptr);

    /**
     * 创建virtual resource(只含有相应的desc，没有实际资源）
     * @param name: resource的名称
     * @param desc: 资源对应的描述符
     * @return 创建的virtual resource对应的handle
     * */
    template<class RESOURCE>
    RenderGraphId<RESOURCE> Create(const std::string& name,
                                   const typename RESOURCE::Descriptor& desc) {
      // todo
    }

    /**
     * 从一个已有的resource派生出sub resource（共用部分属性，仅特化部分属性）
     * @param parent:根resource
     * @param name:名称
     * @param desc:sub resource的描述符
     * @return 派生出的sub resource的handle
     * */
    template<class RESOURCE>
    RenderGraphId<RESOURCE> CreateSubResource(RenderGraphId<RESOURCE> parent,
                                              const std::string& name,
                                              const typename RESOURCE::SubDescriptor& desc = {}) {
      // todo
    }

    /**
     * 将一个resource标记为读，会添加一条从该pass到resource到边
     * 标志该pass会读取该resource
     * @param handle:要读取到resource
     * @param usage:resource会怎样被使用
     * @return 返回resource的handle，之前传入到handle会失效
     * */
    template<class RESOURCE>
    inline RenderGraphId<RESOURCE> Read(RenderGraphId<RESOURCE> handle,
                                        typename RESOURCE::Usage usage = RESOURCE::DEFAULT_R_USAGE) {
      // todo
    }


    /**
     * 建立一个从该pass到resource到写访问，
     * 添加一条从resource到pass的边
     * @param handle:要写入的resource
     * @param usage:resource会怎么被使用
     * @return resource的新handle，传入的handle会失效
     * */
    template<class RESOURCE>
    inline RenderGraphId<RESOURCE> Write(RenderGraphId<RESOURCE> handle,
                                         typename RESOURCE::Usage usage = RESOURCE::DEFAULT_W_USAGE) {
      // todo
    }

    /**
     * 将该pass标记为一个叶节点，即不会被裁剪掉
     * 对一个ExternalResource调用Write会自动添加SideEffect以防止其被裁剪
     * */
    void SideEffect() noexcept;

    /**
     * 获取resource所关联对desc
     * @return 所使用对descriptor
     * */
    template<class RESOURCE>
    const typename RESOURCE::Descriptor& GetDescriptor(RenderGraphId<RESOURCE> handle) const {
      // todo
    }

    //! 获取resource的name
    std::string GetName(RenderGraphHandle handle) const;


    /**
     * 创建texture资源（与其他资源区分）
     * @param desc：texture的描述符
     * @param name: 资源名
     * @return: 创建所得的texture的handle
     * */
    RenderGraphId<RenderGraphTexture> CreateTexture(const std::string& name,
                                                    const RenderGraphTexture::Descriptor& desc = {}) noexcept {
      // todo
    }

    /**
     * 针对需要sample的texture的特化创建函数
     * 会自动给texture添加sample的usage
     * @param handle： 原始的texture
     * @return： 添加sample usage之后的handle，原handle会失效
     * */
    RenderGraphId<RenderGraphTexture> Sample(RenderGraphId<RenderGraphTexture> handle) noexcept {
      // todo
    }

   private:
    friend class RenderGraph;
    Builder(RenderGraph& rg, PassNode* node) noexcept;
    ~Builder() noexcept = default;
    RenderGraph& render_graph_;
    PassNode* const pass_node_;
  };


  explicit RenderGraph(ResourceAllocatorInterface& allocator_interface);
  ~RenderGraph() noexcept;

  RenderGraph(const RenderGraph&) = delete;
  RenderGraph& operator=(const RenderGraph&) = delete;

  Blackboard& GetBlackboard() noexcept { return blackboard_;}
  const Blackboard& GetBlackboard() const noexcept { return blackboard_;}

  /**
   * 添加一个pass
   * @tparam DATA:包含pass所需数据的结构
   * @tparam SETUP：lambda,用于装载数据，[](Builder&, DATA&)
   * @tparam EXECUTE：lambda，用于实际的执行（包含对backend的调用） [](const RenderGraphResources&, const DATA&, Driver*)
   *
   * @param name:pass 名称
   * @param setup:描述数据如何装载，同步调用，通常在此声明资源
   * @param execute：实际的执行，异步调用，故capture需要传值
   *
   * @return: 生成的pass的引用
   * */
  template<typename DATA, typename SETUP, typename EXECUTE>
  RenderGraphPass<DATA, EXECUTE>& AddPass(const std::string& name, SETUP setup, EXECUTE&& execute);

  /**
   * 创建一个简单的执行pass，
   * note: 通过该接口创建的pass不会被裁剪掉
   *
   * @tparam EXECUTE:lambda [](Driver*)
   * @param execute：异步调用，需capture值
   * */
  template<class EXECUTE>
  void AddTrivialSideEffectPass(const std::string&name,
                                EXECUTE&& execute);


  /**
   * 分配资源以及裁剪一些无用的节点
   * */
  RenderGraph& Compile() noexcept;

  /**
   * 执行所有的pass
   * */
  void Execute(Driver* driver) noexcept;

  /**
   * 用已有的resource去替换另一个resource
   * @param handle：要替换的resource
   * @param replaced： 要被替换的resource
   * */
  template<class RESOURCE>
  RenderGraphId<RESOURCE> ForwardResource(RenderGraphId<RESOURCE> handle,
                                          RenderGraphId<RESOURCE> replaced);

  /**
   * 设置resource为要展示的资源
   * 会给其添加一个保护，防止其被裁剪
   * */
  template<class RESOURCE>
  void Present(RenderGraphId<RESOURCE> resource);


  /**
   * 从外部导入资源。该导入的资源不会受到render graph的管理，
   * 即其声明周期仍然由外部管理
   * */
  template<class RESOURCE>
  RenderGraphId<RESOURCE> Import(const std::string& name,
                                 const typename RESOURCE::Descriptor& desc,
                                 typename RESOURCE::Usage usage,
                                 const RESOURCE& resource) noexcept;

  /**
   * 将一个render target作为Texture导入到render graph
   * */
  RenderGraphId<RenderGraphTexture> Import(const std::string& name,
                                           const RenderGraphRenderPassInfo::ExternalDescriptor& desc,
                                           RenderTargetHandle render_target);

  /**
   * 检查handle是否可用
   * */
  bool IsValid(RenderGraphHandle handle) const;

  /**
   * 检查在Compile之后该pass是否被裁剪
   * */
  bool IsCulled(const RenderGraphPassBase& pass) const noexcept;


  /**
   * 获得resource对应的descriptor
   * */
  template<class RESOURCE>
  const typename RESOURCE::Descriptor& GetDescriptor(RenderGraphId<RESOURCE> handle) const {
    // todo
  }

  //! 检查是否有环
  bool IsAcyclic() const noexcept;

  void ExportGraphviz(std::ostream& out, const std::string& name = "");
 private:
  friend class RenderGraphResources;
  friend class PassNode;
  friend class ResourceNode;
  friend class RenderPassNode;
  struct Empty{};

  DependencyGraph& GetGraph() { return graph_;}
  ResourceAllocatorInterface& GetResourceAllocator() noexcept {return resource_allocator_;}


  //! 该结构体用于描述一个resource的信息
  struct ResourceSlot {
    using Version = RenderGraphHandle::Version;
    using Index = int64_t;
    Index rid = 0; // resources_中的id
    Index nid = 0; // resource_nodes_中的id
    Index sid = -1; // 该id只在sub resource中使用，记录了parent的idx, todo:解释
    Version version = 0;
  };
  void Reset() noexcept;
  void AddPresentPass(std::function<void(Builder&)> setup) noexcept;
  Builder AddPassInternal(const std::string& name, RenderGraphPassBase* base) noexcept;

  /**
   * 在现有resource的基础上，创建一个新的resource，并将所有原handle指向该新resource
   * 即  o(handle)   ====>   o-----dup_node(handle)
   * @param handle：要创建新resource的handle
   * @return 返回新resource的handle（原handle未失效，直接指向该新resource）
   * */
  RenderGraphHandle CreateNewVersion(RenderGraphHandle handle, RenderGraphHandle parent = {}) noexcept;
  /**
   * 将现有resource转换为新的resource（拷贝）的subresource，并将所有原handle指向新的parent
   * 即  s(handle)   ===>    s->parent(handle)
   * */
  RenderGraphHandle CreateNewVersionForSubresourceIfNeeded(RenderGraphHandle handle) noexcept;

  RenderGraphHandle AddResourceInternal(VirtualResource* resource) noexcept;
  RenderGraphHandle AddSubResourceInternal(RenderGraphHandle parent, VirtualResource* sub) noexcept;
  RenderGraphHandle ReadInternal(RenderGraphHandle handle, PassNode* pass,
                                 std::function<bool(ResourceNode*, VirtualResource*)> connect);
  RenderGraphHandle WriteInternal(RenderGraphHandle handle, PassNode* pass,
                                  std::function<bool(ResourceNode*, VirtualResource*)> connect);
  RenderGraphHandle ForwardResourceInternal(RenderGraphHandle handle,
                                            RenderGraphHandle replaced_handle);

 private:
  template<class RESOURCE>
  RenderGraphId<RESOURCE> Create(const std::string& name,
                                 const typename RESOURCE::Descriptor& desc) noexcept;

  template<class RESOURCE>
  RenderGraphId<RESOURCE> CreateSubresource(RenderGraphId<RESOURCE> parent,
                                            const std::string& name,
                                            const typename RESOURCE::SubDescriptor& desc) noexcept;

  template<class RESOURCE>
  RenderGraphId<RESOURCE> Read(PassNode* pass,
                               RenderGraphId<RESOURCE> resource,
                               typename RESOURCE::Usage usage);

  template<class RESOURCE>
  RenderGraphId<RESOURCE> Write(PassNode* pass,
                                RenderGraphId<RESOURCE> resource,
                                typename RESOURCE::Usage usage);

  ResourceSlot& GetResourceSlot(RenderGraphHandle handle) noexcept {
    if (handle.index_ >=  resource_slots_.size() ||
        resource_slots_[handle.index_].rid >= resources_.size() ||
        resource_slots_[handle.index_].nid >= resource_nodes_.size()) {
      assert(false);
    }
    return resource_slots_[handle.index_];
  }

  const ResourceSlot& GetResourceSlot(RenderGraphHandle handle) const noexcept {
    return const_cast<RenderGraph*>(this)->GetResourceSlot(handle);
  }

  VirtualResource* GetResource(RenderGraphHandle handle) noexcept {
    if (!handle.IsInitialized()) {
      assert(false);
      return nullptr;
    }
    const ResourceSlot& slot = GetResourceSlot(handle);
    if (slot.rid >= resources_.size()) {
      assert(false);
      return nullptr;
    }
    return resources_[slot.rid];
  }

  ResourceNode* GetActiveResourceNode(RenderGraphHandle handle) noexcept {
    const ResourceSlot& slot = GetResourceSlot(handle);
    if (slot.nid >= resource_nodes_.size()) {
      assert(false);
      return nullptr;
    }
    return resource_nodes_[slot.nid];
  }


  const VirtualResource* GetResource(RenderGraphHandle handle) const noexcept {
    return const_cast<RenderGraph*>(this)->GetResource(handle);
  }
  const ResourceNode* GetResourceNode(RenderGraphHandle handle) const noexcept {
    return const_cast<RenderGraph*>(this)->GetActiveResourceNode(handle);
  }
  void DestroyInternal();

 private:
  Blackboard blackboard_;
  ResourceAllocatorInterface& resource_allocator_;
  DependencyGraph graph_;

  std::vector<ResourceSlot> resource_slots_;
  std::vector<VirtualResource*> resources_;
  std::vector<ResourceNode*> resource_nodes_;
  std::vector<PassNode*> pass_nodes_;
  // 该iter用于标识active和非active的节点的分界
  // 在cull后，该iter之前的节点为active的节点
  std::vector<PassNode*>::iterator active_pass_end_;
  //! 已分配的空间
  using Allocator = utils::SimpleAllocator;

  Allocator allocator_;
};

template<typename DATA, typename SETUP, typename EXECUTE>
RenderGraphPass<DATA, EXECUTE> &RenderGraph::AddPass(const std::string &name,
                                                                    SETUP setup,
                                                                    EXECUTE &&execute) {
  static_assert(sizeof(EXECUTE) < 1024, "Execute lambda is too large");

  const auto* pass = allocator_.Make<RenderGraphPass<DATA, EXECUTE>>(std::forward<EXECUTE>(execute));
  Builder builder(AddPassInternal(name, pass));
  setup(builder, const_cast<DATA&>(pass->GetData()));

  return *pass;
}

template<typename EXECUTE>
void RenderGraph::AddTrivialSideEffectPass(const std::string &name, EXECUTE &&execute) {
  AddPass<Empty>(name, [](RenderGraph::Builder& builder, auto&) {builder.SideEffect();},
                 [execute](const RenderGraphResources&, const auto&, Driver* driver) {
    execute(driver);
  });
}

template<typename RESOURCE>
void RenderGraph::Present(RenderGraphId<RESOURCE> resource) {
  //! 不会添加任何usage flags, 只是添加read标识，防止被裁剪
  AddPresentPass([&](Builder& builder) { builder.Read(resource, {}); });
}

template<typename RESOURCE>
RenderGraphId<RESOURCE> RenderGraph::Create(const std::string &name,
                                            const typename RESOURCE::Descriptor &desc) noexcept {
  auto* resource =allocator_.Make<Resource<RESOURCE>>(name, desc);
  VirtualResource* vresource(resource);

  return RenderGraphId<RESOURCE>(AddResourceInternal(vresource));
}


template<typename RESOURCE>
RenderGraphId<RESOURCE> RenderGraph::CreateSubresource(RenderGraphId<RESOURCE> parent,
                                                       const std::string &name,
                                                       const typename RESOURCE::SubDescriptor &desc) noexcept {
  auto* parent_resource = static_cast<Resource<RESOURCE>*>(GetResource(parent));
  VirtualResource* v_resource(allocator_.Make<Resource<RESOURCE>>(parent_resource, name, desc));
  return RenderGraphId<RESOURCE>(AddSubResourceInternal(parent, v_resource));
}


template<typename RESOURCE>
RenderGraphId<RESOURCE> RenderGraph::Import(const std::string &name,
                                                      const typename RESOURCE::Descriptor &desc,
                                                      typename RESOURCE::Usage usage,
                                                      const RESOURCE &resource) noexcept {
  VirtualResource* v_resource(allocator_.Make<ExternalResource<RESOURCE>>(name, desc, usage, resource));
  
  return RenderGraphId<RESOURCE>(AddResourceInternal(v_resource));
}

template<class RESOURCE>
RenderGraphId<RESOURCE> RenderGraph::Read(PassNode *pass,
                                          RenderGraphId<RESOURCE> resource,
                                          typename RESOURCE::Usage usage) {
  RenderGraphId<RESOURCE> result(ReadInternal(resource, pass,
                                              [this, pass, usage]
                                              (ResourceNode* node, VirtualResource* vrsrc) {
    Resource<RESOURCE>* resource = static_cast<Resource<RESOURCE>*>(vrsrc);
    return resource->Connect(graph_, node, pass, usage);
  }));
  return result;
}

template<class RESOURCE>
RenderGraphId<RESOURCE> RenderGraph::Write(PassNode *pass,
                                           RenderGraphId<RESOURCE> resource,
                                           typename RESOURCE::Usage usage) {
  RenderGraphId<RESOURCE> result(WriteInternal(resource, pass,
                                               [this, pass, usage]
                                               (ResourceNode* node, VirtualResource* vrsrc) {
    Resource<RESOURCE>* resource = static_cast<Resource<RESOURCE>*>(vrsrc);
    return resource->Connect(graph_, pass, node, usage);
  }));
  return result;
}

template<class RESOURCE>
RenderGraphId<RESOURCE> RenderGraph::ForwardResource(
    RenderGraphId<RESOURCE> handle,
    RenderGraphId<RESOURCE> replaced) {
  return RenderGraphId<RESOURCE>(ForwardResourceInternal(handle, replaced));
}



}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPH_H_
