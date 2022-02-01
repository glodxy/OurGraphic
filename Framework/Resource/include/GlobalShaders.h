//
// Created by glodxy on 2022/1/31.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_GLOBALSHADERS_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_GLOBALSHADERS_H_
#include <map>

#include "Backend/include/Handle.h"
#include "include/GlobalEnum.h"
#include "Backend/include/Driver.h"
namespace our_graph {
/**
 * 该类用于管理一些通用的全局shader
 * 1. 用于延迟光照light pass的shader
 * */
class GlobalShaders {
 public:
  static GlobalShaders& Get() {
    static GlobalShaders global_shaders;
    return global_shaders;
  }
  void Init(Driver* driver);
  // 获取对应的global shader
  ShaderHandle GetGlobalShader(GlobalShaderType shader) const;
 private:
  GlobalShaders() = default;
  void InitDeferredLight();

  Driver* driver_ = nullptr;
  std::map<GlobalShaderType, ShaderHandle> shaders_ = {};
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_GLOBALSHADERS_H_
