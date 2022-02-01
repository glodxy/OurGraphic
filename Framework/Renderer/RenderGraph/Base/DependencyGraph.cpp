//
// Created by Glodxy on 2022/1/11.
//

#include "DependencyGraph.h"
#include "Utils/OGLogging.h"


namespace our_graph::render_graph {

DependencyGraph::DependencyGraph() noexcept {
  nodes_.reserve(8);
  edges_.reserve(16);
}

DependencyGraph::~DependencyGraph() noexcept = default;

uint32_t DependencyGraph::GenerateNodeId() noexcept {
  return nodes_.size();
}

void DependencyGraph::RegisterNode(Node *node, NodeID id) noexcept {
  // 要求必须要是新的node
  if (id!= nodes_.size()) {
    LOG_ERROR("DependencyGraph", "node[{}] error!, current size:{}",
              id, nodes_.size());
    return;
  }

  NodeContainer& nodes = nodes_;
  if (nodes.capacity() == nodes.size()) {
    nodes.reserve(nodes.capacity() * 2);
  }
  nodes.push_back(node);
}

bool DependencyGraph::IsEdgeValid(const Edge *edge) const noexcept {
  auto& nodes = nodes_;
  const Node* from = nodes[edge->from];
  const Node* to = nodes[edge->to];
  // 都没被裁剪则可用
  return !from->IsCulled() && !to->IsCulled();
}

void DependencyGraph::Link(Edge *edge) noexcept {
  EdgeContainer& edges = edges_;
  if (edges.capacity() == edges.size()) {
    edges.reserve(edges.capacity() * 2);
  }
  edges.push_back(edge);
}

const DependencyGraph::EdgeContainer &DependencyGraph::GetEdges() const noexcept {
  return edges_;
}

const DependencyGraph::NodeContainer &DependencyGraph::GetNodes() const noexcept {
  return nodes_;
}

DependencyGraph::EdgeContainer DependencyGraph::GetIncomingEdges(const Node *node) const noexcept {
  auto result = EdgeContainer();
  result.reserve(edges_.size());
  const NodeID node_id = node->GetID();
  std::copy_if(edges_.begin(), edges_.end(),
               std::back_insert_iterator<EdgeContainer>(result),
               [node_id](auto edge) {return edge->to == node_id;});
  return result;
}

DependencyGraph::EdgeContainer DependencyGraph::GetOutgoingEdges(const Node *node) const noexcept {
  auto result = EdgeContainer();
  result.reserve(edges_.size());
  const NodeID node_id = node->GetID();
  std::copy_if(edges_.begin(), edges_.end(),
               std::back_insert_iterator<EdgeContainer>(result),
               [node_id](auto edge) {return edge->from == node_id;});
  return result;
}

DependencyGraph::Node *DependencyGraph::GetNode(NodeID id) noexcept {
  return nodes_[id];
}

const DependencyGraph::Node *DependencyGraph::GetNode(NodeID id) const noexcept {
  return nodes_[id];
}

void DependencyGraph::Cull() noexcept {
  auto& nodes = nodes_;
  auto& edges = edges_;

  // 将所有读取的ref count+1
  for (const Edge* p_edge : edges) {
    Node* node = nodes[p_edge->from];
    node->ref_count_++;
  }

  // 找到所有的0ref的节点
  auto stack = NodeContainer();
  stack.reserve(nodes.size());
  for (Node* const p_node : nodes) {
    if (p_node->GetRefCount() == 0) {
      stack.push_back(p_node);
    }
  }

  while (!stack.empty()) {
    Node* const p_node = stack.back();
    stack.pop_back();
    // 找到该节点的所有入边(读）
    // 即该节点读取了哪些资源
    const EdgeContainer & incoming = GetIncomingEdges(p_node);
    for (Edge* edge : incoming) {
      // 找到该边的读取资源
      Node* linked_node = GetNode(edge->from);
      if (-- linked_node->ref_count_ == 0) {
        stack.push_back(linked_node);
      }
    }
  }
}

void DependencyGraph::Clear() noexcept {
  edges_.clear();
  nodes_.clear();
}

void DependencyGraph::ExportGraphviz(std::ostream &out, const std::string &name) {
#ifdef DEBUG
  std::string graph_name = name.empty() ? name : "graph";
  out << "digraph \"" << graph_name << "\" {\n";
  out << "rankdir = LR\n";
  out << "bgcolor = black\n";
  out << "node [shape=rectangle, fontname=\"helvetica\", fontsize=10]\n\n";

  auto const& nodes = nodes_;

  for (Node const* node : nodes) {
    uint32_t id = node->GetID();
    std::string s = node->Graphvizify();
    out << "\"N" << id << "\" " << s.c_str() << "\n";
  }

  out << "\n";
  for (Node const* node : nodes) {
    uint32_t id = node->GetID();

    auto edges = GetOutgoingEdges(node);
    auto first = edges.begin();
    auto pos = std::partition(first, edges.end(),
                              [this](auto const& edge) { return IsEdgeValid(edge); });

    std::string s = node->GraphvizifyEdgeColor();

    // render the valid edges
    if (first != pos) {
      out << "N" << id << " -> { ";
      while (first != pos) {
        Node const* ref = GetNode((*first++)->to);
        out << "N" << ref->GetID() << " ";
      }
      out << "} [color=" << s.c_str() << "2]\n";
    }

    // render the invalid edges
    if (first != edges.end()) {
      out << "N" << id << " -> { ";
      while (first != edges.end()) {
        Node const* ref = GetNode((*first++)->to);
        out << "N" << ref->GetID() << " ";
      }
      out << "} [color=" << s.c_str() << "4 style=dashed]\n";
    }
  }

  out << "}" << std::endl;
#endif
}

bool DependencyGraph::IsAcyclic() const noexcept {
#ifdef DEBUG
  DependencyGraph graph;
  graph.edges_ = edges_;
  graph.nodes_ = nodes_;
  return DependencyGraph::IsAcyclicInternal(graph);
#endif
}

bool DependencyGraph::IsAcyclicInternal(DependencyGraph &graph) noexcept {
#ifdef DEBUG
  while (!graph.nodes_.empty() && !graph.edges_.empty()) {
    auto pos = std::find_if(graph.nodes_.begin(), graph.nodes_.end(),
                            [&graph](const Node* node) {
      // 查找所有的边，看是否有边使用该节点
      auto pos = std::find_if(graph.edges_.begin(), graph.edges_.end(),
                              [node](const Edge* edge) {
        return edge->from == node->GetID();
      });
      // 无，代表这是个叶子节点，返回
      return pos == graph.edges_.end();
    });
    // 没有叶子节点，代表有环
    if (pos == graph.nodes_.end()) {
      return false;
    }

    // 移除所有使用该节点的边
    auto last = std::remove_if(graph.edges_.begin(), graph.edges_.end(),
                               [&pos](const Edge* edge) {
      return edge->to == (*pos)->GetID() || edge->from == (*pos)->GetID();
    });

    // 移除该节点
    graph.nodes_.erase(pos);
  }
  return true;
#endif
}


/*-----------Node------------*/
DependencyGraph::Node::Node(DependencyGraph &graph) noexcept
  : id_(graph.GenerateNodeId()) {
  graph.RegisterNode(this, id_);
}

// 有交集返回1
uint32_t DependencyGraph::Node::GetRefCount() const noexcept {
  return (ref_count_ & TARGET) ? 1 : ref_count_;
}

void DependencyGraph::Node::MakeTarget() noexcept {
  if (ref_count_ != 0 && ref_count_ != TARGET) {
    LOG_ERROR("Node", "MakeTarget Error! ref:{}", ref_count_);
    assert(false);
  }
  ref_count_ = TARGET;
}

const char *DependencyGraph::Node::GetName() const noexcept {
  return "unknown_node";
}

std::string DependencyGraph::Node::Graphvizify() const noexcept {
#ifdef DEBUG
  std::string s;
  s.reserve(128);

  uint32_t id = GetID();
  const char* const nodeName = GetName();
  uint32_t const refCount = GetRefCount();

  s.append("[label=\"");
  s.append(nodeName);
  s.append("\\nrefs: ");
  s.append(std::to_string(refCount));
  s.append(", id: ");
  s.append(std::to_string(id));
  s.append("\", style=filled, fillcolor=");
  s.append(refCount ? "skyblue" : "skyblue4");
  s.append("]");
  s.shrink_to_fit();

  return s;
#else
  return {};
#endif
}

std::string DependencyGraph::Node::GraphvizifyEdgeColor() const noexcept {
#ifdef DEBUG
  return std::string("darkolivegreen");
#else
  return {};
#endif
}

}  // namespace our_graph::render_graph