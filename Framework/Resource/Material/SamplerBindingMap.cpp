//
// Created by Glodxy on 2021/12/5.
//

#include "SamplerBindingMap.h"
#include "Framework/Backend/include/DriverEnum.h"
#include "Framework/Resource/Material/SamplerBlock.h"
#include "Framework/include/GlobalEnum.h"
#include "Framework/Resource/Material/SamplerBlockGenerator.h"
namespace our_graph {

void SamplerBindingMap::Init(const SamplerBlock *block, const std::string &material_name) {
  // 该offset即每个block的起始位置
  uint8_t offset = 0;
  size_t max_sampler_idx = MAX_SAMPLER_COUNT - 1;
  bool overflow = false;
  for (uint8_t block_idx = 0; block_idx < BindingPoints::COUNT; ++block_idx) {
    sampler_block_offsets_[block_idx] = offset;
    const SamplerBlock * real_block;
    if (block_idx == BindingPoints::PER_MATERIAL_INSTANCE) {
      // 如果是每个material instance都有的block，则进行绑定
      // 即用户自定义的与material相关的sampler block
      real_block = block;
    } else {
      // 对于非用户定义的材质相关sampler，需要实时生成
      real_block = SamplerBlockGenerator::GenerateSamplerBlock(BindingPoints(block_idx), 0);
    }
    if (real_block) {
      auto fields = real_block->GetSamplerInfoList();
      for (const auto& field : fields) {
        // 拥有的sampler数大于支持的最大sampler数
        // 将overflow设为true，方便后面处理
        if (offset > max_sampler_idx) {
          overflow = true;
        }
        AddSampler({
          .block_index = block_idx,
          .local_offset = field.offset,
          .global_offset = offset++
        });
      }
    }
  }

  if (overflow) {
    // 处理溢出的情况
    // 打印所有成功设置的sampler
    LOG_WARN("SamplerBindingMap", "mat[{}] Overflow!", material_name);
    offset = 0;
    for (uint8_t block_idx = 0; block_idx < BindingPoints::COUNT; ++block_idx) {
      const SamplerBlock * real_block;
      if (block_idx == BindingPoints::PER_MATERIAL_INSTANCE) {
        real_block = block;
      } else {
        // 对于非用户定义的材质相关sampler，需要实时生成
        real_block = SamplerBlockGenerator::GenerateSamplerBlock(BindingPoints(block_idx), 0);
      }
      if (real_block) {
        auto fields = real_block->GetSamplerInfoList();
        for (const auto& field : fields) {
          LOG_INFO("SamplerBindingMap", ">>>Setup Sampler:{}", field.name);
          offset++;
        }
      }
    }
  }
}

void SamplerBindingMap::AddSampler(SamplerBindingInfo info) {
  // 如果该sampler的offset要比这个block的首个sampler位置小，则更新该block的首个sampler offset
  if (info.global_offset < sampler_block_offsets_[info.block_index]) {
    sampler_block_offsets_[info.block_index] = info.global_offset;
  }
  binding_map_[GetBindingKey(info.block_index, info.local_offset)] = info;
}


}