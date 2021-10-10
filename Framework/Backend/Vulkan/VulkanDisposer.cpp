//
// Created by Glodxy on 2021/10/10.
//
#include "../include_internal/DriverEnum.h"
#include "VulkanDisposer.h"
#include "Utils/OGLogging.h"

namespace {
static constexpr uint32_t FRAMES_BEFORE_EVICTION = our_graph::MAX_COMMAND_BUFFERS_COUNT;
}

namespace our_graph {
void VulkanDisposer::CreateDisposable(Key resource, DestructFunc func) noexcept {
  if (disposables_.find(resource) == disposables_.end()) {
    disposables_.insert(std::make_pair(resource,Disposable()));
  }
  disposables_[resource].destructor = func;
}

void VulkanDisposer::RemoveReference(Key resource) noexcept {
  if (!resource) {
    return;
  }
  auto iter = disposables_.find(resource);
  if (iter != disposables_.end() && iter->second.ref_count > 0) {
    --disposables_[resource].ref_count;
  }
}

/**
 * 每次请求都会刷新该资源的最近使用时间
 * */
void VulkanDisposer::Acquire(Key resource) noexcept {
  if (!resource) {
    return;
  }

  auto iter = disposables_.find(resource);
  if (iter == disposables_.end()) {
    return;
  }
  Disposable& value = iter->second;

  if (value.ref_count <= 0 || value.ref_count >= 65535) {
    return;
  }

  // 如果当前没有在使用，则加1
  if (value.remain_frames == 0) {
    ++value.ref_count;
  }
  value.remain_frames = FRAMES_BEFORE_EVICTION;
}

void VulkanDisposer::GC() noexcept {
  for (auto iter : disposables_) {
    Disposable& value = iter.second;
    if (value.ref_count > 0 && value.remain_frames > 0) {
      if (--value.remain_frames == 0) {
        RemoveReference(iter.first);
      }
    }
  }

  decltype(disposables_) tmp_disposables;
  for (auto iter : disposables_) {
    Disposable& value = iter.second;
    if (value.ref_count == 0) {
      value.destructor();
    } else {
      tmp_disposables.insert({iter.first, value});
    }
  }
  disposables_.swap(tmp_disposables);
}

void VulkanDisposer::Reset() noexcept {
  LOG_INFO("VulkanDisposable", "Remove {} disposables",
           disposables_.size());

  for (auto iter : disposables_) {
    iter.second.destructor();
  }
  disposables_.clear();
}

}  // namespace our_graph