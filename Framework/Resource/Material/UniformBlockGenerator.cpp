//
// Created by Glodxy on 2021/12/22.
//

#include "Framework/Resource/Material/UniformBlockGenerator.h"
#include "Framework/Resource/Material/UniformBlock.h"
namespace our_graph {
const UniformBlock * UniformBlockGenerator::GetUniformBlock(BindingPoints binding_points) {
  switch (binding_points) {
    case BindingPoints::PER_VIEW:
      return &GeneratePerViewBlock();
    default:
      return nullptr;
  }
}

const UniformBlock & UniformBlockGenerator::GeneratePerViewBlock() {
  static UniformBlock block;
  return block;
}

}