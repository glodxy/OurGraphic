//
// Created by Glodxy on 2022/1/15.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_PASS_RENDERGRAPHPASS_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_PASS_RENDERGRAPHPASS_H_
#include "Backend/include/Driver.h"
#include "Renderer/RenderGraph/Resource/RenderGraphResources.h"
namespace our_graph::utils {
class SimpleAllocator;
}
namespace our_graph::render_graph {

/**
 * 该类为各个pass的具体执行类基类
 * */
class RenderGraphPassExecutor {
  friend class RenderGraph;
  friend class PassNode;
  friend class RenderPassNode;
 public:
  RenderGraphPassExecutor() noexcept = default;
  virtual ~RenderGraphPassExecutor() noexcept;
  RenderGraphPassExecutor(const RenderGraphPassExecutor&) = delete;
  RenderGraphPassExecutor& operator=(const RenderGraphPassExecutor&) = delete;

 protected:
  virtual void Execute(const RenderGraphResources& resources, Driver* driver) = 0;
};

class RenderGraphPassBase : protected RenderGraphPassExecutor {
  friend class RenderGraph;
  friend class PassNode;
  friend class RenderPassNode;
 public:
  using RenderGraphPassExecutor::RenderGraphPassExecutor;
  virtual ~RenderGraphPassBase() noexcept;
 protected:
  PassNode* node_ = nullptr;
  void SetNode(PassNode* node) noexcept {node_ = node;}
  const PassNode* GetNode() const noexcept {return node_;}
};

template<typename DATA, typename EXECUTE>
class RenderGraphPass : public RenderGraphPassBase {
  friend class RenderGraph;
  friend class utils::SimpleAllocator;
 public:
  const DATA& GetData() const noexcept {return data_;}
  const DATA* operator->() const {return &GetData();}
 protected:
  explicit RenderGraphPass(EXECUTE&& exe) noexcept
  : RenderGraphPassBase(), execute_(std::move(exe)) {

  }

  void Execute(const RenderGraphResources& resources, Driver* driver) noexcept final {
    execute_(resources, data_, driver);
  }


  EXECUTE execute_;
  DATA data_;
};

}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_PASS_RENDERGRAPHPASS_H_
