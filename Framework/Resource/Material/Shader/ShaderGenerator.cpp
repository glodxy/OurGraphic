//
// Created by Glodxy on 2021/12/22.
//
#include "ShaderGenerator.h"
#include <sstream>

#include "CodeGenerator.h"
#include "Framework/Backend/include/Program.h"
#include "Framework/Resource/Material/ShaderCache.h"
#include "Framework/Resource/Material/UniformBlockGenerator.h"
#include "Framework/Resource/Material/SamplerBlockGenerator.h"
namespace our_graph {
std::string ShaderGenerator::CreateShaderText(ShaderType type,
                                              const MaterialInfo &material_info,
                                              uint32_t module_key, uint8_t subpass_idx) {
  switch (type) {
    case ShaderType::VERTEX:
      return CreateVertexShader(material_info, module_key, subpass_idx);
    case ShaderType::FRAGMENT:
      return CreateFragShader(material_info, module_key, subpass_idx);
  }
}

std::string ShaderGenerator::CreateGlobalShaderText(ShaderType type,
                                                    uint32_t moduile_key,
                                                    GlobalShaderType shader_type) {
  switch (type) {
    case ShaderType::VERTEX:
      return CreateGlobalVertexShader(moduile_key, shader_type);
    case ShaderType::FRAGMENT:
      return CreateGlobalFragShader(moduile_key, shader_type);
  }
}

std::string ShaderGenerator::CreateGlobalVertexShader(uint32_t module_key,
                                                      GlobalShaderType shader_type) {
  std::stringstream ss;
  CodeGenerator cg(ss, ShaderType::VERTEX);


  cg.GenerateHead();
  cg.GenerateDefine("SHADER_TYPE_VERTEX", true);
  // 生成uniform
  cg.GenerateUniforms(BindingPoints::PER_VIEW, *UniformBlockGenerator::GetUniformBlock(BindingPoints::PER_VIEW));

  cg.GenerateSeparator();

  // 生成内置模块内容
  cg.AppendCode(ShaderCache::GetModuleContent(module_key));

  switch (shader_type) {
    case GlobalShaderType::DEFERRED_LIGHT: {
      cg.AppendCode(ShaderCache::GetDataFromFile(GLOBAL_SHADER_FILES[GlobalShaderFileType::TEXTUREQUAD_VS]));
      break;
    }
  }
  return ss.str();
}

std::string ShaderGenerator::CreateGlobalFragShader(uint32_t module_key,
                                                    GlobalShaderType shader_type) {
  std::stringstream ss;
  CodeGenerator cg(ss, Program::ShaderType::FRAGMENT);

  cg.GenerateHead();
  cg.GenerateDefine("SHADER_TYPE_FRAGMENT", true);
  // 生成uniform
  cg.GenerateUniforms(BindingPoints::PER_VIEW, *UniformBlockGenerator::GetUniformBlock(BindingPoints::PER_VIEW));
  cg.GenerateUniforms(BindingPoints::LIGHT, *UniformBlockGenerator::GetUniformBlock(BindingPoints::LIGHT));


  cg.GenerateSeparator();

  // 生成sampler
  auto per_view_sampler = SamplerBlockGenerator::GenerateSamplerBlock(BindingPoints::PER_VIEW, module_key);
  if (per_view_sampler) {
    cg.GenerateSamplers(0,
                        *per_view_sampler);
  }

  // 生成内置模块内容
  cg.AppendCode(ShaderCache::GetModuleContent(module_key));


  switch (shader_type) {
    case GlobalShaderType::DEFERRED_LIGHT: {
      cg.AppendCode(ShaderCache::GetDataFromFile(GLOBAL_SHADER_FILES[GlobalShaderFileType::DEFERRED_LIGHT_PASS_FS]));
      break;
    }
  }
  return ss.str();
}


std::string ShaderGenerator::CreateVertexShader(const MaterialInfo &material_info,
                                                uint8_t module_key, uint8_t subpass_idx) {
  using ShaderType = Program::ShaderType;
  std::stringstream ss;
  CodeGenerator cg(ss, ShaderType::VERTEX);


  cg.GenerateHead();
  // 生成定义
  cg.GenerateDefine(GetShadingModelDefine(material_info.shading_model), true);
  cg.GenerateDefine("HAS_MATERIAL", true);
  cg.GenerateDefine("SHADER_TYPE_VERTEX", true);
  // 生成render path
  cg.GenerateRenderPath(material_info.render_path);
  // 生成property的定义
  cg.GenerateMaterialProperties(material_info.property_list);
  // 1 生成输入(此处实际只生成了定义)
  cg.GenerateShaderInput(material_info.required_attributes);
  // 2 接着生成实际的shader输入
  cg.AppendCode(ShaderCache::GetVsInputData());

  // 生成模块输入
  //cg.AppendCode(ShaderCache::GetModuleInput(module_key));

  // 生成变量
  uint32_t idx = 0;
  for (const auto& variable : material_info.variant_list) {
    cg.GenerateVariable(variable.name, variable.type, variable.size, idx++);
  }

  // 生成uniform
  cg.GenerateUniforms(BindingPoints::PER_VIEW, *UniformBlockGenerator::GetUniformBlock(BindingPoints::PER_VIEW));
  cg.GenerateUniforms(BindingPoints::PER_RENDERABLE, *UniformBlockGenerator::GetUniformBlock(BindingPoints::PER_RENDERABLE));
  cg.GenerateUniforms(BindingPoints::PER_MATERIAL_INSTANCE, material_info.uniform_block);

  cg.GenerateSeparator();

  // 生成sampler
  cg.GenerateSamplers(material_info.sampler_binding_map.GetBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE),
                      material_info.sampler_block);
  // 添加Getter
  cg.AppendCode(ShaderCache::GetGetterData());
  // todo:内置material属性
  // 生成material的输入
  cg.AppendCode(ShaderCache::GetMaterialInputVsData());
  // 生成默认模块函数
  cg.AppendCode(ShaderCache::GetModuleContent(module_key));

  // 添加material的shader内容
  cg.AppendCode(ShaderCache::GetDataFromFile(material_info.pass_list[subpass_idx].vertex_shader));

  // 添加main
  cg.AppendCode(ShaderCache::GetBasePassVsData());
  return ss.str();
}

std::string ShaderGenerator::CreateFragShader(const MaterialInfo &material_info,
                                              uint8_t module_key, uint8_t subpass_idx) {
  std::stringstream ss;
  CodeGenerator cg(ss, Program::ShaderType::FRAGMENT);

  cg.GenerateHead();
  // 生成定义
  cg.GenerateDefine("SHADER_TYPE_FRAGMENT", true);
  cg.GenerateDefine(GetShadingModelDefine(material_info.shading_model), true);
  cg.GenerateDefine("HAS_MATERIAL", true);
  // 生成render path
  cg.GenerateRenderPath(material_info.render_path);
  // 生成宏定义
  cg.GenerateDefine("HAS_REFRACTION", material_info.refraction_mode == RefractionMode::NONE);
  // 生成property的定义
  cg.GenerateMaterialProperties(material_info.property_list);
  // todo:折射

  // todo:根据module_key生成宏定义

  // 生成双面材质的宏定义
  cg.GenerateDefine("MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY", material_info.has_double_sided_capability);
  // 生成混合模式的宏定义
  switch (material_info.blending_mode) {
    case BlendingMode::OPAQUE:
      cg.GenerateDefine("BLEND_MODE_OPAQUE", true);
      break;
    case BlendingMode::TRANSPARENT:
      cg.GenerateDefine("BLEND_MODE_TRANSPARENT", true);
      break;
    case BlendingMode::ADD:
      cg.GenerateDefine("BLEND_MODE_ADD", true);
      break;
    case BlendingMode::MASKED:
      cg.GenerateDefine("BLEND_MODE_MASKED", true);
      break;
    case BlendingMode::FADE: {
      // 该类型是半透明的特殊处理
      cg.GenerateDefine("BLEND_MODE_TRANSPARENT", true);
      cg.GenerateDefine("BLEND_MODE_FADE", true);
      break;
    }
    case BlendingMode::MULTIPLY:
      cg.GenerateDefine("BLEND_MODE_MULTIPLY", true);
      break;
    case BlendingMode::SCREEN:
      cg.GenerateDefine("BLEND_MODE_SCREEN", true);
      break;
  }
  switch (material_info.post_lighting_blending_mode) {
    case BlendingMode::OPAQUE:
      cg.GenerateDefine("POST_LIGHTING_BLEND_MODE_OPAQUE", true);
      break;
    case BlendingMode::TRANSPARENT:
      cg.GenerateDefine("POST_LIGHTING_BLEND_MODE_TRANSPARENT", true);
      break;
    case BlendingMode::ADD:
      cg.GenerateDefine("POST_LIGHTING_BLEND_MODE_ADD", true);
      break;
    case BlendingMode::MULTIPLY:
      cg.GenerateDefine("POST_LIGHTING_BLEND_MODE_MULTIPLY", true);
      break;
    case BlendingMode::SCREEN:
      cg.GenerateDefine("POST_LIGHTING_BLEND_MODE_SCREEN", true);
      break;
  }


  // 判断是否使用了自定义的表面着色
  cg.GenerateDefine("MATERIAL_CUSTOM_SURFACE_SHADING", material_info.has_custom_surface_shading);

  // 生成material的输入(此处仅生成控制宏)
  cg.GenerateShaderInput(material_info.required_attributes);
  // 添加实际的输入
  cg.AppendCode(ShaderCache::GetFsInputData());

  // 生成模块的输入
  //cg.AppendCode(ShaderCache::GetModuleInput(module_key));

  uint32_t idx = 0;
  for (const auto& variable : material_info.variant_list) {
    cg.GenerateVariable(variable.name, variable.type, variable.size, idx++);
  }

  // 生成uniform
  cg.GenerateUniforms(BindingPoints::PER_VIEW, *UniformBlockGenerator::GetUniformBlock(BindingPoints::PER_VIEW));
  cg.GenerateUniforms(BindingPoints::PER_RENDERABLE, *UniformBlockGenerator::GetUniformBlock(BindingPoints::PER_RENDERABLE));
  cg.GenerateUniforms(BindingPoints::LIGHT, *UniformBlockGenerator::GetUniformBlock(BindingPoints::LIGHT));
  cg.GenerateUniforms(BindingPoints::PER_MATERIAL_INSTANCE, material_info.uniform_block);

  cg.GenerateSeparator();

  // 生成sampler
  auto per_view_sampler = SamplerBlockGenerator::GenerateSamplerBlock(BindingPoints::PER_VIEW, module_key);
  if (per_view_sampler) {
    cg.GenerateSamplers(material_info.sampler_binding_map.GetBlockOffset(BindingPoints::PER_VIEW),
                        *per_view_sampler);
  }
  cg.GenerateSamplers(material_info.sampler_binding_map.GetBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE),
                      material_info.sampler_block);

  // 生成Getter
  cg.AppendCode(ShaderCache::GetGetterData());
  // 生成material的输入
  cg.AppendCode(ShaderCache::GetMaterialInputFsData());
  // 生成内置模块内容
  cg.AppendCode(ShaderCache::GetModuleContent(module_key));
  // 生成material的shader
  cg.AppendCode(ShaderCache::GetDataFromFile(material_info.pass_list[subpass_idx].frag_shader));

  // 添加main
  cg.AppendCode(ShaderCache::GetBasePassFsData());
  return ss.str();
}

std::string ShaderGenerator::GetShadingModelDefine(ShadingModel model) {
  switch (model) {
    case ShadingModel::LIT: return "SHADING_MODEL_LIT";
    case ShadingModel::UNLIT: return "SHADING_MODEL_UNLIT";
    case ShadingModel::SUBSURFACE: return "SHADING_MODEL_SUBSURFACE";
  }
}

}  // namespace our_graph