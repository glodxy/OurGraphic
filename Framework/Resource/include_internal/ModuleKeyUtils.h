//
// Created by Glodxy on 2022/2/3.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_INTERNAL_MODULEKEYUTILS_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_INTERNAL_MODULEKEYUTILS_H_
#include "include/GlobalEnum.h"
namespace our_graph {
class ModuleKeyUtils {
 public:
  // 判断有没有动态光照
  static bool HasDynamicLight(uint32_t module_key) {
    return module_key & ShaderVariantBit::DYNAMIC_LIGHTING;
  }
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_INTERNAL_MODULEKEYUTILS_H_
