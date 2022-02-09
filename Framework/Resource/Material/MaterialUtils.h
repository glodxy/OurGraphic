//
// Created by Glodxy on 2022/2/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_MATERIALUTILS_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_MATERIALUTILS_H_
#include <cstdint>
namespace our_graph {
class Program;
class SamplerBlock;
class SamplerBindingMap;
class MaterialUtils {
 public:
  /**
 * 将block中的所有sampler生成sasmpler group
 * @note: 因为每个block并没有限制绑定位置，
 * 所以需手动传入其相应的binding_point
 * */
  static void AddSamplerGroup(Program& program, uint8_t binding_point,
                              const SamplerBlock& block,
                              SamplerBindingMap const& map);
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_MATERIALUTILS_H_
