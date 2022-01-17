//
// Created by Glodxy on 2022/1/16.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPH_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPH_H_
#include <cstdint>
#include <string>

#include "Renderer/RenderGraph/Pass/RenderGraphRenderPass.h"
#include "Renderer/RenderGraph/Base/RenderGraphId.h"
#include "Renderer/RenderGraph/Base/Blackboard.h"
#include "Renderer/RenderGraph/Base/DependencyGraph.h"
#include "Renderer/RenderGraph/Pass/RenderGraphPass.h"

namespace our_graph::render_graph {

class ResourceAllocatorInterface;

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
     * @return 该render pass的idx
     * */
    uint32_t DeclareRenderPass(const std::string& name,
                               const RenderGraphRenderPassInfo::Descriptor& desc);


    /**
     * 快速生成一个color的attachment的render pass
     * @param color：所需要绑定的attachment
     * @param color：返回的idx
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
    const PassNode* pass_node_;
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
                                 const typename RESOURCE::Usage usage,
                                 const RESOURCE& resource) noexcept;

  /**
   * 将一个render target作为Texture导入到render graph
   * */
  RenderGraphId<RenderGraphTexture> Import(const std::string& name,
                                           const RenderGraphRenderPassInfo::ExternalDescriptor& desc,
                                           RenderTargetHandle render_target);
 private:
  Blackboard blackboard_;
  ResourceAllocatorInterface& resource_allocator_;
  DependencyGraph& graph_;

  std::vector<VirtualResource> resources_;
  std::vector<ResourceNode*> resource_nodes_;
  std::vector<PassNode*> pass_nodes_;
};
}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPH_H_
