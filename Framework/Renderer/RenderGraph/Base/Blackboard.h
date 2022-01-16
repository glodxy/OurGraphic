//
// Created by Glodxy on 2022/1/16.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_BLACKBOARD_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_BLACKBOARD_H_
#include <unordered_map>
#include <string>

#include "Renderer/RenderGraph/Base/RenderGraphId.h"
namespace our_graph::render_graph {

/**
 * 该类用于数据的中转
 * */
class Blackboard {
  using Container = std::unordered_map<std::string, RenderGraphHandle>;
 public:
  Blackboard() noexcept;
  ~Blackboard() noexcept;

  RenderGraphHandle& operator[](const std::string& name) noexcept;

  void Put(const std::string& name, RenderGraphHandle handle) noexcept;

  template<class T>
  RenderGraphId<T> Get(const std::string& name) const noexcept {
    return static_cast<RenderGraphId<T>>(GetHandle(name));
  }

  void Remove(const std::string& name) noexcept;
 private:
  RenderGraphHandle GetHandle(const std::string name) const noexcept;
  Container map_;
};
}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_BLACKBOARD_H_
