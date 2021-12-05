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
enum BindingPoints :uint8_t {
  PER_VIEW = 0, // 每次视图更新所使用的uniform/sampler插槽
  PER_RENDERABLE = 1, // 每个物体更新所使用的uniform/sampler
  PER_MATERIAL_INSTANCE, // 每个材质的uniform/sampler
  COUNT
};
}
// 限制前端部分使用的插槽不超过后端部分定义的插槽
static_assert(BindingPoints::COUNT <= CONFIG_BINDING_COUNT);

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_GLOBALENUM_H_
