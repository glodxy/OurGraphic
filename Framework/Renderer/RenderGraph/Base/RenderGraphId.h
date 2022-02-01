//
// Created by Glodxy on 2022/1/13.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RENDERGRAPHID_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RENDERGRAPHID_H_

#include <cstddef>

namespace our_graph::render_graph {

template<class T>
class RenderGraphId;

class RenderGraphHandle {
 public:
  using Index = uint16_t;
  using Version = uint16_t;

 public:
  RenderGraphHandle(const RenderGraphHandle&) = default;
  RenderGraphHandle(RenderGraphHandle&& r)
  : index_(r.index_), version_(r.version_) {
    r.Clear();
  }
  RenderGraphHandle& operator=(const RenderGraphHandle&) = default;
  RenderGraphHandle& operator=(RenderGraphHandle&& r) {
    std::swap(r.index_, index_);
    std::swap(r.version_, version_);
    return *this;
  }

  bool IsInitialized() const noexcept {return index_ != UNINITIALIZED;}
  operator bool() const noexcept {return IsInitialized();}

  void Clear() noexcept {index_ = UNINITIALIZED; version_ = 0;}

  bool operator<(const RenderGraphHandle& r) const noexcept {
    return index_ < r.index_;
  }

  bool operator==(const RenderGraphHandle& r) const noexcept {
    return (index_ == r.index_);
  }

  bool operator!=(const RenderGraphHandle& r) const noexcept {
    return !operator==(r);
  }

  Index GetIndex() const {
    return index_;
  }

  Version GetVersion() const {
    return version_;
  }
 private:
  template<class T>
  friend class RenderGraphId;
  friend class RenderGraph;
  friend class Blackboard;

  RenderGraphHandle() noexcept = default;
  explicit RenderGraphHandle(Index index) noexcept : index_(index) {}

  static constexpr uint16_t UNINITIALIZED = std::numeric_limits<Index>::max();
  Index index_ = UNINITIALIZED;  // 资源的索引
  Version version_ = 0;

};

template<class Resource>
class RenderGraphId : public RenderGraphHandle {
 public:
  using RenderGraphHandle::RenderGraphHandle;
  RenderGraphId() noexcept = default;
  explicit RenderGraphId(RenderGraphHandle r) : RenderGraphHandle(r) {}
};

}
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RENDERGRAPHID_H_
