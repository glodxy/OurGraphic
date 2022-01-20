//
// Created by Glodxy on 2022/1/13.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_PASSNODE_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_PASSNODE_H_
#include "Backend/include/Driver.h"

#include "Renderer/RenderGraph/Pass/RenderGraphPass.h"
#include "Renderer/RenderGraph/Base/Resource.h"
#include "Renderer/RenderGraph/Base/DependencyGraph.h"
#include "Renderer/RenderGraph/Base/RenderGraphId.h"
#include "Renderer/RenderGraph/RenderGraph.h"
#include <unordered_set>
namespace our_graph::render_graph {
class RenderGraphResources;


class PassNode : public  DependencyGraph::Node {
 public:
  using NodeID = DependencyGraph::NodeID;

  PassNode(RenderGraph& rg) noexcept;
  PassNode(PassNode&& r) noexcept;
  PassNode(const PassNode&) = delete;
  PassNode& operator=(const PassNode&) = delete;

  ~PassNode() noexcept override;

  // 注册要使用的资源
  void RegisterResource(RenderGraphHandle resource_handle) noexcept;

  virtual void Execute(const RenderGraphResources& resources, Driver* driver) noexcept = 0;

  virtual void Resolve() noexcept = 0;

  std::string GraphvizifyEdgeColor() const noexcept override;

  std::vector<VirtualResource*> devirtualize_;  // 需要在执行前实例化的资源
  std::vector<VirtualResource*> destroy_;  // 需要在执行后销毁的资源
 protected:
  friend class RenderGraphResourcees;
  RenderGraph& render_graph_;
  // 所有已声明的handle
  std::unordered_set<RenderGraphHandle::Index> declared_handles_;
};

/**
 * 渲染节点，用于向attachment中写入数据
 * 即管理render target相关
 * */
class RenderPassNode : public PassNode {
 public:
  class RenderPassData {
   public:
    static constexpr size_t ATTACHMENT_COUNT = MAX_SUPPORTED_RENDER_TARGET_COUNT + 2;
    std::string name_ = {};
    RenderGraphRenderPassInfo::Descriptor descriptor_;
    bool external_ = false;
    TargetBufferFlags target_buffer_flags_ = {};
    RenderGraphId<RenderGraphTexture> attachment_info_[ATTACHMENT_COUNT] = {};

    // 输入的attachment
    ResourceNode* incoming_[ATTACHMENT_COUNT] = {};
    // 输出的attachment
    ResourceNode* outgoing_[ATTACHMENT_COUNT] = {};

    // backend的数据
    struct {
      RenderTargetHandle target_;
      RenderPassParams params_;
    } backend_;


    void Devirtualize(RenderGraph& rg, ResourceAllocatorInterface& resource_allocator) noexcept;
    void Destroy(ResourceAllocatorInterface& resource_allocator) noexcept;
  };

  RenderPassNode(RenderGraph& rg, const std::string& name, RenderGraphPassBase* base) noexcept;
  RenderPassNode(RenderPassNode&& r) noexcept;
  ~RenderPassNode() noexcept override;

  /**
   * 声明所使用的render target
   * @return : 返回该rendertarget对应的idx
   * */
  uint32_t DeclareRenderTarget(RenderGraph& rg, RenderGraph::Builder& builder,
                               const std::string& name,
                               const RenderGraphRenderPassInfo::Descriptor& desc) noexcept;

  const RenderPassData* GetRenderPassData(uint32_t id) const noexcept;

  const char* GetName() const noexcept override { return name_.c_str();}
 private:

  std::string Graphvizify() const noexcept override;
  void Execute(const RenderGraphResources &resources, Driver *driver) noexcept override;
  void Resolve() noexcept override;

  std::string name_;
  std::unique_ptr<RenderGraphPassBase> pass_base_;

  // setup阶段的render target数据
  std::vector<RenderPassData> render_target_data_;
};

class PresentPassNode : public PassNode {
 public:
  PresentPassNode(RenderGraph& rg) noexcept;
  PresentPassNode(PresentPassNode&& r) noexcept;

  ~PresentPassNode() noexcept override;
  PresentPassNode(const PresentPassNode&) = delete;
  PresentPassNode& operator=(const PresentPassNode&) = delete;

  void Execute(const RenderGraphResources &resources, Driver *driver) noexcept override;
  void Resolve() noexcept override;

 private:
  const char* GetName() const noexcept override;
  std::string Graphvizify() const noexcept override;
};
}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_PASSNODE_H_
