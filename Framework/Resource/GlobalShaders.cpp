//
// Created by glodxy on 2022/1/31.
//

#include "Framework/Resource/include/GlobalShaders.h"
#include "Resource/Material/Shader/ShaderGenerator.h"
#include "Framework/Resource/Material/MaterialUtils.h"
#include "Resource/Material/ShaderCache.h"

namespace our_graph {
void GlobalShaders::Init(Driver* driver) {
  driver_ = driver;
  // 1. deferred light
  InitDeferredLight();
}

void GlobalShaders::InitDeferredLight() {
  UniformBlock::Builder uniform_builder;
  SamplerBlock::Builder sampler_builder;
  // 构建gbuffer的sampler
  sampler_builder.Name("gBuffer");
  for (int i = 0; i < DeferredLightInputBinding::MAX; ++i) {
    std::string name = "A";
    name[0] += i;
    sampler_builder.Add(name,
                        SamplerType::SAMPLER_2D, SamplerFormat::FLOAT);
  }

  UniformBlock uniform = uniform_builder.Build();
  SamplerBlock sampler = sampler_builder.Build();
  SamplerBindingMap sampler_binding_map;
  sampler_binding_map.Init(&sampler);

  uint32_t deferred_light_bit = ShaderVariantBit::DEFERRED_LIGHT |
      ShaderVariantBit::DIRECTIONAL_LIGHTING |
      ShaderVariantBit::DYNAMIC_LIGHTING |
      ShaderVariantBit::BRDF;
  ShaderGenerator sg;
  std::string vs_text = sg.CreateGlobalShaderText(Program::ShaderType::VERTEX, deferred_light_bit, GlobalShaderType::DEFERRED_LIGHT, uniform, sampler, sampler_binding_map);
  std::vector<uint32_t> deferred_light_vs = ShaderCache::CompileFile("deferred_light_vs", Program::ShaderType::VERTEX,
                                                                     vs_text, false);

  std::string fs_text = sg.CreateGlobalShaderText(Program::ShaderType::FRAGMENT, deferred_light_bit, GlobalShaderType::DEFERRED_LIGHT, uniform, sampler, sampler_binding_map);
  std::vector<uint32_t> deferred_light_fs = ShaderCache::CompileFile("deferred_light_fs", Program::ShaderType::FRAGMENT,
                                                                     fs_text, false);


  Program program;
  program.Diagnostics("DeferredLight", deferred_light_bit)
      .WithVertexShader(deferred_light_vs.data(), deferred_light_vs.size() * 4)
      .WithFragmentShader(deferred_light_fs.data(), deferred_light_fs.size() * 4);
  // 配置shader的sampler
  MaterialUtils::AddSamplerGroup(program, BindingPoints::PER_MATERIAL_INSTANCE, sampler, sampler_binding_map);



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