//
// Created by Glodxy on 2021/11/15.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_MODULEBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_MODULEBASE_H_

namespace our_graph {
/**
 * 该类为所有pipeline中流程模块的基类，
 * 包括：rasterizer、test等
 * */
class ModuleBase {
 public:
  void SetContext(void* context) {context_ = context;}
 protected:
  void* context_ {nullptr};
};

}
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_MODULEBASE_H_
