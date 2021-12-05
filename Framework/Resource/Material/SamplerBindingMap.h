//
// Created by Glodxy on 2021/12/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SAMPLERBINDINGMAP_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SAMPLERBINDINGMAP_H_
#include <cstddef>
#include <cassert>
#include <string>
#include <unordered_map>
namespace our_graph {
class SamplerBlock;
/**
 * 该结构体描述了单个sampler在shader中的绑定信息
 * 因为会将所有SamplerBlock都传入GPU
 * */
struct SamplerBindingInfo {
  /**
   * sampler所属的block（也标识了一个对应的binding point)
   * 因为每个block都与一个binding point绑定
   * 该binding point实际标志了该sampler的作用，并非gpu的实际绑定
   * */
  uint8_t block_index;
  /**
   * 该sampler在该block中的偏移量
   * 即在该block中的idx
   * */
  uint8_t local_offset;
  /**
   * 该sampler的binding point
   * @note 与block_index的区别在于：
   * block_index是指该sampler所属block的binding point（仅CPU处使用标识其用途）
   * 而global_offset是指该sampler自己的binding point（GPU中的实际绑定）
   * */
  uint8_t global_offset;
};

/**
 * 该类的作用在于
 * 利用<BlockIndex, LocalOffset> 来查询对应的<GlobalOffset, GroupIndex>
 * */
class SamplerBindingMap {
 public:

  /**
   * 将指定的sampler绑定到可用的binding point
   * @param block_index：该sampler所属的block
   * @param local_offset：该sampler在block中的索引
   * @param global_offset：得到的可用binding point，绑定后返回
   * */
  bool GetSamplerBinding(uint8_t block_index,
                         uint8_t local_offset,
                         uint8_t* global_offset) const {
    // global_offset不能传空
    assert(global_offset);

  }

 private:
  constexpr static uint8_t  UNKNOWN_OFFSET = 0xff;
  /**
   * BindingKey由两部分构成：
   * 1. 前24位用于存储block index
   * 2. 后8位为local offset
   * */
  using BindingKey = uint32_t;
  static BindingKey GetBindingKey(uint8_t block_index, uint8_t local_offset) {
    return ((uint32_t) block_index <<8) + local_offset;
  }

  std::unordered_map<BindingKey, SamplerBindingInfo> binding_map_;
  uint8_t sampler_block_offsets[BindingPoint]
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SAMPLERBINDINGMAP_H_
