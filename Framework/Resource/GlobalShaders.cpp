//
// Created by glodxy on 2022/1/31.
//

#include "Framework/Resource/include/GlobalShaders.h"
#include "Resource/Material/Shader/ShaderGenerator.h"
#include "Resource/Material/ShaderCache.h"

namespace our_graph {
void GlobalShaders::Init(Driver* driver) {
  driver_ = driver;
  // 1. deferred light
  InitDeferredLight();
}

void GlobalShaders::InitDeferredLight() {
  uint32_t deferred_light_bit = ShaderVariantBit::DEFERRED_LIGHT |
      ShaderVariantBit::DIRECTIONAL_LIGHTING |
      ShaderVariantBit::DYNAMIC_LIGHTING |
      ShaderVariantBit::BRDF;
  ShaderGenerator sg;
  std::string vs_text = sg.CreateGlobalShaderText(Program::ShaderType::VERTEX, deferred_light_bit, GlobalShaderType::DEFERRED_LIGHT);
  std::vector<uint32_t> deferred_light_vs = ShaderCache::CompileFile("deferred_light_vs", Program::ShaderType::VERTEX,
                                                                     vs_text, false);

  std::string fs_text = sg.CreateGlobalShaderText(Program::ShaderType::FRAGMENT, deferred_light_bit, GlobalShaderType::DEFERRED_LIGHT);
  std::vector<uint32_t> deferred_light_fs = ShaderCache::CompileFile("deferred_light_fs", Program::ShaderType::FRAGMENT,
                                                                     fs_text, false);
  // todo:调整sampler
  Program::Sampler samplers[DeferredLightInputBinding::MAX];
  for (int i = 0; i < DeferredLightInputBinding::MAX; ++i) {
    samplers[i].name = std::string("gBuffer") + char('A' + i);
    samplers[i].binding = i;
    samplers[i].strict = true;
  }
  Program program;
  program.Diagnostics("DeferredLight", deferred_light_bit)
      .WithVertexShader(deferred_light_vs.data(), deferred_light_vs.size() * 4)
      .WithFragmentShader(deferred_light_fs.data(), deferred_light_fs.size() * 4)
      .SetSamplerGroup(0, samplers, DeferredLightInputBinding::MAX);

  ShaderHandle deferred_light_handle = driver_->CreateShader(std::move(program));
  shaders_[GlobalShaderType::DEFERRED_LIGHT] = deferred_light_handle;
}

our_graph::ShaderHandle our_graph::GlobalShaders::GetGlobalShader(GlobalShaderType shader) const {
  if (shaders_.find(shader) == shaders_.end()) {
    LOG_ERROR("GlobalShaders", "Cannot find global shader:{}, "
                               "maybe not initialized!", shader);
    return {};
  }
  return shaders_.at(shader);
}

}  // namespace our_graph