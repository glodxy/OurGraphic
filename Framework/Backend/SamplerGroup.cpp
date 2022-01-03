//
// Created by Glodxy on 2021/10/10.
//

#include "Framework/Backend/include/SamplerGroup.h"

namespace our_graph {
SamplerGroup::SamplerGroup(size_t count) noexcept : buffer_(count){
}

SamplerGroup::SamplerGroup(const SamplerGroup &r) noexcept = default;

SamplerGroup::SamplerGroup(SamplerGroup &&r) noexcept :
      buffer_(r.buffer_), dirty_(r.dirty_) {
  r.CleanDirty();
}

SamplerGroup &SamplerGroup::operator=(const SamplerGroup &r) noexcept = default;
SamplerGroup &SamplerGroup::operator=(SamplerGroup &&r) noexcept {
  if (this != &r) {
    buffer_ = r.buffer_;
    dirty_ = r.dirty_;
    r.CleanDirty();
  }
  return *this;
}

SamplerGroup &SamplerGroup::SetSamplers(const SamplerGroup &r) noexcept {
  if (this != &r) {
    buffer_ = r.buffer_;
    dirty_ = ((1u << r.buffer_.size())- 1u);
  }
  return *this;
}

void SamplerGroup::SetSampler(size_t index, Sampler sampler) noexcept {
  if (index < buffer_.size()) {
    buffer_[index] = sampler;
    dirty_.set(index);
  }
}

SamplerGroup SamplerGroup::CopyAndClean() const {
  // todo
  return *this;
}



}  // namespace our_graph