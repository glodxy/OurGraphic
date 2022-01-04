//
// Created by Glodxy on 2021/12/22.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMBLOCKGENERATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMBLOCKGENERATOR_H_
#include "Framework/include/GlobalEnum.h"
#include "Framework/Resource/Material/UniformBlock.h"
#include "Framework/Resource/include/UniformStruct.h"
namespace our_graph {

/**
 * 该类用于生成固定的uniform block结构来方便shader等资源的生成
 * */
class UniformBlockGenerator {
 public:
  static const UniformBlock* GetUniformBlock(BindingPoints binding_points) {
    switch (binding_points) {
      case BindingPoints::PER_VIEW:
        return &GeneratePerViewBlock();
      case BindingPoints::PER_RENDERABLE:
        return &GeneratePerRenderableBlock();
      case BindingPoints::LIGHT:
        return &GenerateLightBlock();
      default:
        return nullptr;
    }
  }

 private:
  static const UniformBlock& GeneratePerViewBlock() {
    static UniformBlock block = UniformBlock::Builder()
        .Name(kPerViewUniformName)
        .Add("viewFromWorldMatrix", 1, UniformBlock::Type::MAT4)
        .Add("worldFromViewMatrix", 1, UniformBlock::Type::MAT4)
        .Add("clipFromViewMatrix", 1, UniformBlock::Type::MAT4)
        .Add("viewFromClipMatrix", 1, UniformBlock::Type::MAT4)
        .Add("clipFromWorldMatrix", 1, UniformBlock::Type::MAT4)
        .Add("worldFromClipMatrix", 1, UniformBlock::Type::MAT4)
        .Add("resolution", 1, UniformBlock::Type::FLOAT4)
        .Add("cameraPosition", 1, UniformBlock::Type::FLOAT3)
        .Add("time", 1, UniformBlock::Type::FLOAT)
        .Build();
    return block;
  }

  static const UniformBlock& GeneratePerRenderableBlock() {
    static UniformBlock block = UniformBlock::Builder()
        .Name(kPerRenderableUniformName)
        .Add("worldFromModelMatrix", 1, UniformBlock::Type::MAT4)
        .Add("worldFromModelNormalMatrix", 1, UniformBlock::Type::MAT3)
        .Build();
    return block;
  }

  static const UniformBlock& GenerateLightBlock() {
    static UniformBlock block = UniformBlock::Builder()
        .Name(kLightUnifromName)
        .Add("lights", CONFIG_MAX_LIGHT_COUNT, UniformBlock::Type::MAT4)
        .Build();
    return block;
  }
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMBLOCKGENERATOR_H_
