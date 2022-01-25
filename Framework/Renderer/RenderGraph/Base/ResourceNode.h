//
// Created by Glodxy on 2022/1/16.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RESOURCENODE_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RESOURCENODE_H_
#include "Renderer/RenderGraph/Base/DependencyGraph.h"
#include "Renderer/RenderGraph/Base/RenderGraphId.h"
namespace our_graph::render_graph {
class PassNode;
class RenderGraph;
class ResourceEdgeBase;

class ResourceNode : public DependencyGraph::Node {
 public:
  /**
   * 从h拷贝一份资源，作为parent的子节点
   * */
  ResourceNode(RenderGraph& eg, RenderGraphHandle h, RenderGraphHandle parent) noexcept;
  ~ResourceNode() noexcept override;

  ResourceNode(const ResourceNode&) = delete;
  ResourceNode& operator=(const ResourceNode&) = delete;

  void AddOutgoingEdge(ResourceEdgeBase* edge) noexcept;
  void SetIncomingEdge(ResourceEdgeBase* edge) noexcept;

  const RenderGraphHandle resource_handle_;

  // 是否有pass node写入该resource node
  bool HasWritePass() const noexcept {
    return write_pass_ != nullptr;
  }

  // 是否有有效的writer
  bool HasActiveWriters() const noexcept;
  // 如果该pass node有写入该pass，则返回相应的edge
  ResourceEdgeBase* GetWriteEdgeForPass(const PassNode* node) const noexcept;
  // 判断是否有从node写入该resource
  bool HasWriteFrom(const PassNode* node) const noexcept;

  // 是否有pass node从该resource读取
  bool HasReaders() const noexcept {
    return !reader_passes_.empty();
  }

  // 是否有任意未被裁剪（真正被使用）的pass node从该resource读取
  bool HasActiveReaders() const noexcept;
  // 获取读取该resource的pass node的edge
  ResourceEdgeBase* GetReaderEdgeForPass(const PassNode* node) const noexcept;

  // 解析该节点在graph中的使用
  void ResolveResourceUsage(DependencyGraph& graph) noexcept;

  ResourceNode* GetParentNode() noexcept;

  // 设置读取resource的parent
  // 此处实际会设置一个从parent到该resource的边
  //! read:parent->this
  void SetParentReadDependency(ResourceNode* parent) noexcept;
  //! write:this->parent
  void SetParentWriteDependency(ResourceNode* parent) noexcept;
  //! forward:this->source
  void SetForwardResourceDependency(ResourceNode* source) noexcept;

  const char * GetName() const noexcept override;

 private:
  std::string Graphvizify() const noexcept override;
  std::string GraphvizifyEdgeColor() const noexcept override;

  RenderGraph& render_graph_;
  // 连接到的render pass
  std::vector<ResourceEdgeBase*> reader_passes_;
  // 将要写入的pass
  ResourceEdgeBase* write_pass_ = nullptr;


  //! parent相关
  RenderGraphHandle parent_handle_;
  DependencyGraph::Edge* parent_read_edge_ = nullptr;
  DependencyGraph::Edge* parent_write_edge_ = nullptr;
  DependencyGraph::Edge* forwarded_edge_ = nullptr;
};
}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RESOURCENODE_H_
