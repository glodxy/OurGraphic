//
// Created by Glodxy on 2021/12/5.
//
/**
 * 该文件用于定义全局的enum以及config常量
 *
 * */
#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_GLOBALENUM_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_GLOBALENUM_H_
#include "Backend/include/DriverEnum.h"

namespace our_graph {

namespace {
/**
 * 该enum用于区分资源在shader中的使用
 * uniform buffer以及sampler的绑定都会借助该enum来找到对应的location 或 binding
 * */
enum BindingPoints :uint8_t {
  PER_VIEW = 0, // 每次视图更新所使用的uniform/sampler插槽
  PER_RENDERABLE = 1, // 每个物体更新所使用的uniform/sampler
  PER_MATERIAL_INSTANCE, // 每个材质的uniform/sampler
  LIGHT, // 用于光照的uniform/sampler
  COUNT
};
}

namespace ShaderVariantBit {
  static constexpr uint8_t DIRECTIONAL_LIGHTING   = 0x01; // directional light present, per frame/world position
  static constexpr uint8_t DYNAMIC_LIGHTING       = 0x02; // point, spot or area present, per frame/world position
  static constexpr uint8_t SHADOW_RECEIVER        = 0x04; // receives shadows, per renderable
  static constexpr uint8_t SKINNING_OR_MORPHING   = 0x08; // GPU skinning and/or morphing
  static constexpr uint8_t DEPTH                  = 0x10; // depth only variants
  static constexpr uint8_t FOG                    = 0x20; // fog
  static constexpr uint8_t VSM                    = 0x40; // variance shadow maps
}
// 限制前端部分使用的插槽不超过后端部分定义的插槽
static_assert(BindingPoints::COUNT <= CONFIG_BINDING_COUNT);

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_GLOBALENUM_H_
