//
// Created by Glodxy on 2021/10/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_SAMPLERGROUP_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_SAMPLERGROUP_H_
#include <array>
#include <bitset>
#include "DriverEnum.h"
#include "Handle.h"
#include "Utils/StaticVector.h"

namespace our_graph {

class SamplerGroup {
 public:
  struct Sampler {
    TextureHandle t;
    SamplerParams s{};
  };

  SamplerGroup() noexcept{}

  explicit SamplerGroup(size_t count) noexcept;

  ~SamplerGroup() noexcept = default;
  /**
   * 允许拷贝，但会使用脏位来记录变化
   * */
  SamplerGroup(const SamplerGroup& r) noexcept;
  SamplerGroup& operator=(const SamplerGroup& r) noexcept;
  SamplerGroup(SamplerGroup&& r) noexcept;
  SamplerGroup& operator= (SamplerGroup&& r) noexcept;

  SamplerGroup& SetSamplers(const SamplerGroup& r) noexcept;

  const Sampler* GetSamplers() const noexcept {
    return buffer_.data();
  }

  size_t GetSize() const noexcept {
    return buffer_.size();
  }

  bool IsDirty() const noexcept {
    return dirty_.any();
  }

  void CleanDirty() {
    dirty_.reset();
  }

  void SetSampler(size_t index, Sampler sampler) noexcept;

  inline void SetSampler(size_t index, TextureHandle t, SamplerParams s) {
    SetSampler(index, {t, s});
  }

  inline void ClearSampler(size_t index) {
    SetSampler(index, Sampler());
  }

 private:
  static_vector<Sampler, MAX_SAMPLER_COUNT> buffer_;
  mutable std::bitset<32> dirty_;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_SAMPLERGROUP_H_
