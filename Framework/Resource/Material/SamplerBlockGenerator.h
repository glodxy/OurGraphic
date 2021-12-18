//
// Created by Glodxy on 2021/12/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SAMPLERBLOCKGENERATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SAMPLERBLOCKGENERATOR_H_
#include "Framework/Resource/Material/SamplerBlock.h"
#include "Framework/include/GlobalEnum.h"
namespace our_graph {
/**
 * 该类的作用在于实时根据需要生成需要的sampler block
 * 以作为实际使用的sampler定义
 * */
class SamplerBlockGenerator {
 public:
  /**
   * 生成需要的sampler block来让material使用
   * @param binding_points：该sampler的类型
   * @param key：标志会使用的shader模块
   * */
  static const SamplerBlock& GenerateSamplerBlock(BindingPoints binding_points, uint32_t key);

 protected:
  static const SamplerBlock& GeneratePerViewBlock(uint32_t key);
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SAMPLERBLOCKGENERATOR_H_