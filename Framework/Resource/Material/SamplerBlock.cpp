//
// Created by Glodxy on 2021/12/5.
//

#include "SamplerBlock.h"

namespace our_graph {
/*-------Builder---------*/
SamplerBlock::Builder &SamplerBlock::Builder::Name(const std::string &name) {
  name_ = name;
  return *this;
}

Builder &SamplerBlock::Builder::Add(const std::string &name, Type type, Format format, bool multisample) {
  entries_.emplace_back(name, type, format, multisample);
  return *this;
}

SamplerBlock SamplerBlock::Builder::Build() {
  return SamplerBlock(*this);
}
/* --------------------*/

SamplerBlock::SamplerBlock(const Builder &builder)
  : name_(builder.name_) {
  auto& info_map = info_map_;
  auto& samplers_list = sampler_info_list_;
  info_map.reserve(builder.entries_.size());
  samplers_list.resize(builder.entries_.size());

  uint32_t i = 0;
  for (const auto& entry : builder.entries_) {
    SamplerInfo& info = samplers_list[i];
    info = {entry.name_, i, entry.type_, entry.format_, entry.multi_sample_};

    info_map[info.name.c_str()] = i;
    ++i;
  }
  size_ = i;
}

const SamplerInfo *SamplerBlock::GetSamplerInfo(const std::string &name) const {
  const auto& iter = info_map_.find(name);
  if (iter == info_map_.end()) {
    LOG_ERROR("SamplerBlock", "{} Not Find!", name);
    return nullptr;
  }
  return &sampler_info_list_[iter->second];
}

std::string SamplerBlock::GetUniformName(const std::string &group, const std::string &sampler) noexcept {
  char uniform_name[256];

  const char* prefix = std::copy_n(sampler,
                                   std::min(sizeof(uniform_name)/2, group.size()), prefix + 1);

}



}  // namespace our_graph