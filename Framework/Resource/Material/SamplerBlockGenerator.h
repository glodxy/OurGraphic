//
// Created by Glodxy on 2021/12/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SAMPLERBLOCKGENERATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SAMPLERBLOCKGENERATOR_H_
#include "Framework/include/GlobalEnum.h"
#include "Framework/Resource/Material/SamplerBlock.h"
namespace our_graph {
/**
 * 该类的作用在于实时根据需要生成需要的sampler block
 * 以作为实际使用的sampler定义
 * */
class SamplerBlockGenerator {
 public:
  /**
   * 获取需要的sampler block来让material使用
   * @param binding_points：该sampler的类型
   * @param key：标志会使用的shader模块
   * */
  static const SamplerBlock* GenerateSamplerBlock(BindingPoints binding_points, uint32_t key) noexcept {
    switch (binding_points) {
      // todo:暂时不支持光照的per view sampler
      case BindingPoints::PER_VIEW : {
        return &GeneratePerViewBlock();
      }
    }
    return nullptr;
  }

  static const SamplerBlock& GeneratePerViewBlock();

};
}
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SAMPLERBLOCKGENERATOR_H_
