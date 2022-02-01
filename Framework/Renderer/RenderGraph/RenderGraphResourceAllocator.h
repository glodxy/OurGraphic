//
// Created by Glodxy on 2022/1/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPHRESOURCEALLOCATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPHRESOURCEALLOCATOR_H_
#include <string>
#include <map>
#include "Renderer/RenderGraph/Base/IResourceAllocator.h"
#include "Backend/include/Driver.h"
#include "Utils/HashUtils.h"
#include "Utils/Container/AssociativeContainer.h"
namespace our_graph::render_graph {

/**
 * 该resource allocator用于在render graph中管理资源
 * */
class RenderGraphResourceAllocator : public ResourceAllocatorInterface {
 public:
  RenderGraphResourceAllocator(Driver* driver);
 public:
  RenderTargetHandle CreateRenderTarget(const std::string &name, TargetBufferFlags targetBufferFlags, uint32_t width, uint32_t height, uint8_t samples, MRT color, TargetBufferInfo depth, TargetBufferInfo stencil) noexcept override;
  TextureHandle CreateTexture(const std::string &name, SamplerType target, uint8_t levels, TextureFormat format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, std::array<TextureSwizzle, 4> swizzle, TextureUsage usage) noexcept override;
  void DestroyRenderTarget(RenderTargetHandle h) noexcept override;
  void DestroyTexture(TextureHandle h) noexcept override;

public:
    //销毁所有的资源
    void Destroy();
    // 垃圾回收
    void GC();
protected:
    // 该key用于标识一个特有的texture
    struct TextureKey {
        std::string name; // texture 名称
        SamplerType sampler_type;
        uint8_t levels;
        TextureFormat format;
        uint8_t samples;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        TextureUsage usage;
        std::array<TextureSwizzle, 4> swizzles;

        size_t GetSize() const;

        bool operator==(const TextureKey& key)const {
            return sampler_type == key.sampler_type &&
                levels == key.levels &&
                format == key.format &&
                samples == key.samples &&
                width == key.width &&
                height == key.height &&
                depth == key.depth &&
                usage == key.usage &&
                swizzles == key.swizzles;
        }

        friend size_t Hash(const TextureKey& key){
          size_t seed = 0;
          utils::CombineHash(seed, key.sampler_type);
          utils::CombineHash(seed, key.levels);
          utils::CombineHash(seed, key.format);
          utils::CombineHash(seed, key.samples);
          utils::CombineHash(seed, key.width);
          utils::CombineHash(seed, key.height);
          utils::CombineHash(seed, key.depth);
          utils::CombineHash(seed, key.usage);
          utils::CombineHash(seed, key.swizzles[0]);
          utils::CombineHash(seed, key.swizzles[1]);
          utils::CombineHash(seed, key.swizzles[2]);
          utils::CombineHash(seed, key.swizzles[3]);
        }
    };

    struct TextureCache {
      TextureHandle handle;
      size_t age = 0; // 使用的周期
      uint32_t size = 0;
    };

  template<class T>
  class Hasher {
    size_t operator()(const T& v) const noexcept{
      return Hash(v);
    }
  };

  template<class T>
  class Hasher<Handle<T>> {
    size_t operator()(const Handle<T>& v) const noexcept {
      std::hash<typename Handle<T>::HandleId> hasher;
      return hasher(v.GetId());
    }
  };


  using TextureCacheContainer = utils::AssociativeContainer<TextureKey, TextureCache>;

  // 销毁指定位置的资源
  TextureCacheContainer::iterator Purge(const TextureCacheContainer::iterator& pos);

  Driver* driver_;
  // 该cache用于存储key与cache之间的关联
  TextureCacheContainer texture_cache_;
  // 该cache用于存储handle与key之间的关联(当前在使用的texture)
  //! 用于反向定位
  utils::AssociativeContainer<TextureHandle, TextureKey> texture_used_keys_;

  // 当前周期
  size_t age_ = 0;
  // 当前的总大小
  size_t cached_size_ = 0;
  static constexpr bool enabled_ = true;
};
} // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_RENDERGRAPHRESOURCEALLOCATOR_H_
