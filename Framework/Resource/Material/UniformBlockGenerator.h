//
// Created by Glodxy on 2021/12/22.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMBLOCKGENERATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMBLOCKGENERATOR_H_
#include "Framework/include/GlobalEnum.h"
namespace our_graph {
class UniformBlock;

/**
 * 该类用于生成固定的uniform block结构来方便shader等资源的生成
 * */
class UniformBlockGenerator {
 public:
  static const UniformBlock* GetUniformBlock(BindingPoints binding_points);

 private:
  static const UniformBlock& GeneratePerViewBlock();
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMBLOCKGENERATOR_H_
