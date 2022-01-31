//
// Created by Home on 2022/1/31.
//

#include "Framework/Resource/include/GlobalShaders.h"
#include "Resource/Material/ShaderCache.h"

namespace our_graph {
std::map<GlobalShaderType, ShaderHandle> GlobalShaders::shaders_ = {};

void GlobalShaders::Init(Driver* driver) {
  driver_ = driver;
  // 1. deferred light
  InitDeferredLight();
}

void GlobalShaders::InitDeferredLight() {
  uint32_t deferred_light_bit = ShaderVariantBit::DEFERRED_LIGHT |
      ShaderVariantBit::DIRECTIONAL_LIGHTING |
      ShaderVariantBit::DYNAMIC_LIGHTING;
  std::vector<uint32_t> deferred_light_vs = ShaderCache::GetCompiledData(
      GLOBAL_SHADER_FILES[GlobalShaderFileType::TEXTUREQUAD_VS],
      deferred_light_bit);
  std::vector<uint32_t> deferred_light_fs = ShaderCache::GetCompiledData(
      GLOBAL_SHADER_FILES[GlobalShaderFileType::DEFERRED_LIGHT_PASS_FS],
      deferred_light_bit);
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
      .SetSamplerGroup(BindingPoints::PER_MATERIAL_INSTANCE, samplers, DeferredLightInputBinding::MAX);

  ShaderHandle deferred_light_handle = driver_->CreateShader(std::move(program));
  shaders_[GlobalShaderType::DEFERRED_LIGHT] = deferred_light_handle;
}


ShaderHandle GlobalShaders::GetGlobalShader(GlobalShaderType shader) {
  if (shaders_.find(shader) == shaders_.end()) {
    LOG_ERROR("GlobalShaders", "Cannot find global shader:{}, "
                               "maybe not initialized!", shader);
    return {};
  }
  return shaders_[shader];
}

}  // namespace our_graph