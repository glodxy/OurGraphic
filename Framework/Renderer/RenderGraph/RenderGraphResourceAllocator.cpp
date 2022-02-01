//
// Created by Glodxy on 2022/1/23.
//

#include "RenderGraphResourceAllocator.h"
#include "Resource/include/Texture.h"
#include "include/GlobalEnum.h"
namespace our_graph::render_graph {
size_t RenderGraphResourceAllocator::TextureKey::GetSize() const {
  size_t pixel_size = width * height * depth;
  size_t size = pixel_size * Texture::GetFormatSize(format);

  // 获取采样数
  size_t s = std::max(uint8_t(1), samples);
  if (s > 1) {
    size *= s;
  }

  // 检查mipmap
  if (levels > 1) {
    // 计算结果是4/3(可见文档)
    size += size / 3;
  }

  return size;
}

RenderGraphResourceAllocator::RenderGraphResourceAllocator(Driver *driver)
    : driver_(driver) {
}

void RenderGraphResourceAllocator::Destroy() {
  if (texture_used_keys_.size() >= 1) {
    LOG_ERROR("RenderGraphResourceAllocator", "texture in used!");
    assert(false);
    return;
  }
  // 清除cache中的所有资源
  auto& texture_cache = texture_cache_;
  for (auto iter = texture_cache.begin(); iter != texture_cache.end();) {
    driver_->DestroyTexture(iter->second.handle);
    iter = texture_cache.erase(iter);
  }
}

RenderTargetHandle RenderGraphResourceAllocator::CreateRenderTarget(const std::string &name,
                                                                    TargetBufferFlags targetBufferFlags,
                                                                    uint32_t width,
                                                                    uint32_t height,
                                                                    uint8_t samples,
                                                                    MRT color,
                                                                    TargetBufferInfo depth,
                                                                    TargetBufferInfo stencil) noexcept {
  LOG_INFO("RenderGraphResourceAllocator", "Create RenderTarget[{}]", name);
  return driver_->CreateRenderTarget(targetBufferFlags, width, height, samples, color, depth, stencil);
}

void RenderGraphResourceAllocator::DestroyRenderTarget(RenderTargetHandle h) noexcept {
  driver_->DestroyRenderTarget(h);
}

TextureHandle RenderGraphResourceAllocator::CreateTexture(const std::string &name,
                                                          SamplerType target,
                                                          uint8_t levels,
                                                          TextureFormat format,
                                                          uint8_t samples,
                                                          uint32_t width,
                                                          uint32_t height,
                                                          uint32_t depth,
                                                          std::array<TextureSwizzle, 4> swizzle,
                                                          TextureUsage usage) noexcept {
  samples = samples ? samples : uint8_t(1);

  constexpr const auto default_swizzle = std::array<TextureSwizzle, 4> {
    TextureSwizzle::CHANNEL_0,
    TextureSwizzle::CHANNEL_1,
    TextureSwizzle::CHANNEL_2,
    TextureSwizzle::CHANNEL_3
  };

  TextureHandle handle;
  // 检查是否启用了cache
  if constexpr(enabled_) {
    auto& texture_cache = texture_cache_;
    const TextureKey key {name, target, levels, format, samples,
                          width, height, depth, usage, swizzle};
    const auto iter = texture_cache.find(key);
    // 如果在cache中，则将其移出
    if (iter != texture_cache.end()) {
      handle = iter->second.handle;
      cached_size_ -= iter->second.size;
      texture_cache.erase(iter);
    } else {
      // 没有则分配
      if (swizzle == default_swizzle) {
        handle = driver_->CreateTexture(target, levels, format, samples, width, height, depth, usage);
      } else {
        LOG_ERROR("RenderGraphResourceAllocator", "not support swizzled texture!");
        assert(false);
      }
    }
    texture_used_keys_.emplace(handle, key);
  } else {
    // 未启用cache，直接构造返回
    if (swizzle == default_swizzle) {
      handle = driver_->CreateTexture(target, levels, format, samples,
                                      width, height, depth, usage);
    } else {
      LOG_ERROR("RenderGraphResourceAllocator", "not support swizzled texture!");
      assert(false);
    }
  }
  return handle;
}

void RenderGraphResourceAllocator::DestroyTexture(TextureHandle h) noexcept {
  // 如果使用了cache，会将资源回收至cache
  if constexpr(enabled_) {
    auto iter = texture_used_keys_.find(h);
    if (iter == texture_used_keys_.end()) {
      LOG_ERROR("RenderGraphResourceAllocator", "cannot find texture in use!");
      assert(false);
      return;
    }

    // 找到对应的key
    const TextureKey key = iter->second;
    uint32_t size = key.GetSize();

    texture_cache_.emplace(key, TextureCache{h, age_, size});
    cached_size_ += size;

    texture_used_keys_.erase(iter);
  } else {
    // 否则直接销毁
    driver_->DestroyTexture(h);
  }
}

void RenderGraphResourceAllocator::GC() {
  const size_t age = age_++;

  auto& texture_cache = texture_cache_;
  for (auto iter = texture_cache.begin(); iter != texture_cache.end();) {
    // 获取经过的周期
    const size_t diff_age = age- iter->second.age;
    // 超过最大限制则移除
    if (diff_age > RG_RESOURCE_CACHE_MAX_AGE) {
      iter = Purge(iter);
      // 缓存的大小未到达指定的容量时，则每次只销毁一个
      if (cached_size_ < RG_RESOURCE_CACHE_CAPACITY) {
        break;
      }
    } else {
      ++iter;
    }
  }

  // 容量超过限制时，重新构建
  if (cached_size_ >= RG_RESOURCE_CACHE_CAPACITY) {
    std::vector<std::pair<TextureKey, TextureCache>> tmp_cache;
    tmp_cache.reserve(texture_cache.size());
    tmp_cache.assign(texture_cache.begin(), texture_cache.end());

    // 按照age的升序排序
    // 即最近最少使用
    std::sort(tmp_cache.begin(), tmp_cache.end(), [](const auto& l, const auto& r) {
      return l.second.age < r.second.age;
    });

    // 移除最近最少使用的
    auto curr = tmp_cache.begin();
    while (cached_size_ >= RG_RESOURCE_CACHE_CAPACITY) {
      Purge(texture_cache.find(curr->first));
      ++curr;
    }

    size_t oldest_age = tmp_cache.front().second.age;
    for (auto& iter : texture_cache) {
      iter.second.age -= oldest_age;
    }
    age_ -= oldest_age;
  }
}

RenderGraphResourceAllocator::TextureCacheContainer::iterator RenderGraphResourceAllocator::Purge(
    const TextureCacheContainer::iterator &pos) {
  driver_->DestroyTexture(pos->second.handle);
  cached_size_ -= pos->second.size;
  return texture_cache_.erase(pos);
}

}  // namespace our_graph::render_graph
