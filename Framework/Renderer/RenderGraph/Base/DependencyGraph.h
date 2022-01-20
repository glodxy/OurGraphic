//
// Created by Glodxy on 2022/1/11.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_DEPENDENCYGRAPH_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_DEPENDENCYGRAPH_H_

#include <cstddef>
#include <vector>
#include <iostream>

namespace our_graph::render_graph {
class DependencyGraph {
 public:
  DependencyGraph() noexcept;
  ~DependencyGraph() noexcept;
  DependencyGraph(const DependencyGraph&) noexcept = delete;
  DependencyGraph& operator=(const DependencyGraph&) noexcept = delete;

  using NodeID = uint32_t;

  class Node;

  struct Edge {
    const NodeID from;
    const NodeID to;

    /**
     * 创建一条边
     * @param graph：所属的graph
     * @param from：起始节点
     * @param to：终止节点
     * */
    Edge(DependencyGraph& graph, Node* from, Node* to);

    // 禁止边的拷贝
    Edge(const Edge& r) noexcept = delete;
    Edge& operator=(const Edge&) noexcept = delete;
  };


  class Node {
   public:
    /**
     * 创建节点并添加到graph
     * */
    Node(DependencyGraph& graph) noexcept;

    // 禁用拷贝
    Node(const Node&) noexcept = delete;
    Node& operator=(const Node&) noexcept = delete;

    Node(Node&&) noexcept = default;
    virtual ~Node() noexcept = default;

    NodeID GetID() const noexcept {return id_;}

    void MakeTarget() noexcept;

    /**
     * 判断节点是否是Target（即目标节点）
     * */
    bool IsTarget() const noexcept { return ref_count_ >= TARGET; }

    /**
     * 判断该节点是否被裁剪
     * */
    bool IsCulled() const noexcept {return ref_count_ == 0;}

    /**
     * 返回该节点的引用数
     * */
    uint32_t GetRefCount() const noexcept;

    virtual const char* GetName() const noexcept;
   private:


    virtual std::string Graphvizify() const noexcept;

    virtual std::string GraphvizifyEdgeColor() const noexcept;

    friend class DependencyGraph;
    static const constexpr uint32_t TARGET = 0x80000000u;
    const NodeID id_;
    uint32_t ref_count_ = 0;
  };

  using EdgeContainer = std::vector<Edge*>;
  using NodeContainer = std::vector<Node*>;

  void Clear() noexcept;
  // 获取所有
  const EdgeContainer& GetEdges() const noexcept;
  const NodeContainer& GetNodes() const noexcept;

  // 获取所有的入边
  EdgeContainer GetIncomingEdges(const Node* node) const noexcept;

  // 获取所有的出边
  EdgeContainer GetOutgoingEdges(const Node* node) const noexcept;

  // 获取指定的node
  const Node* GetNode(NodeID id) const noexcept;
  Node* GetNode(NodeID id) noexcept;

  // 裁剪无用的节点
  void Cull() noexcept;


  bool IsEdgeValid(const Edge* edge) const noexcept;
  void ExportGraphviz(std::ostream& out,
                      const std::string& name);

  // 判断有无环
  bool IsAcyclic() const noexcept;

 private:
  uint32_t GenerateNodeId() noexcept;
  void RegisterNode(Node* node, NodeID id) noexcept;
  void Link(Edge* edge) noexcept;
  static bool IsAcyclicInternal(DependencyGraph& graph) noexcept;
  NodeContainer nodes_;
  EdgeContainer edges_;
};

inline DependencyGraph::Edge::Edge(DependencyGraph &graph, Node *from, Node *to) :
from(from->GetID()), to(to->GetID()) {
  graph.Link(this);
}
}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_DEPENDENCYGRAPH_H_
