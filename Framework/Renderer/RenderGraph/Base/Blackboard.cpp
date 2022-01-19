//
// Created by Glodxy on 2022/1/16.
//

#include "Blackboard.h"

namespace our_graph::render_graph {

Blackboard::Blackboard() noexcept = default;
Blackboard::~Blackboard() noexcept = default;

RenderGraphHandle Blackboard::GetHandle(const std::string name) const noexcept {
  auto iter = map_.find(name);
  if (iter == map_.end()) {
    return {};
  }
  return iter->second;
}

RenderGraphHandle &Blackboard::operator[](const std::string &name) noexcept {
  auto [iter, _] = map_.insert_or_assign(name, RenderGraphHandle{});
  return iter->second;
}

void Blackboard::Put(const std::string &name, RenderGraphHandle handle) noexcept {
  operator[](name) = handle;
}

void Blackboard::Remove(const std::string &name) noexcept {
  map_.erase(name);
}

}  // namespace our_graph::render_graph