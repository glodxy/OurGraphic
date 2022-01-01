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
                                              uint32_t module_key) {
  switch (type) {
    case ShaderType::VERTEX:
      return CreateVertexShader(material_info, module_key);
    case ShaderType::FRAGMENT:
      return CreateFragShader(material_info, module_key);
  }
}


std::string ShaderGenerator::CreateVertexShader(const MaterialInfo &material_info, uint8_t module_key) {
  using ShaderType = Program::ShaderType;
  std::stringstream ss;
  CodeGenerator cg(ss, ShaderType::VERTEX);


  cg.GenerateHead();
  // 生成定义
  cg.GenerateDefine(GetShadingModelDefine(material_info.shading_model), true);
  // 生成输入
  cg.GenerateShaderInput(material_info.required_attributes);
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

  // todo:内置material属性

  // 生成默认模块函数
  cg.AppendCode(ShaderCache::GetModuleContent(module_key));

  // 添加material的shader内容
  cg.AppendCode(ShaderCache::GetDataFromFile(material_info.vertex_shader_file));

  return ss.str();
}

std::string ShaderGenerator::CreateFragShader(const MaterialInfo &material_info, uint8_t module_key) {
  std::stringstream ss;
  CodeGenerator cg(ss, Program::ShaderType::FRAGMENT);

  cg.GenerateHead();

  // 生成宏定义
  cg.GenerateDefine("HAS_REFRACTION", material_info.refraction_mode == RefractionMode::NONE);
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

  // 生成定义
  cg.GenerateDefine(GetShadingModelDefine(material_info.shading_model), true);

  // 判断是否使用了自定义的表面着色
  cg.GenerateDefine("MATERIAL_CUSTOM_SURFACE_SHADING", material_info.has_custom_surface_shading);

  // 生成material的输入
  cg.GenerateShaderInput(material_info.required_attributes);

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
  cg.GenerateSamplers(material_info.sampler_binding_map.GetBlockOffset(BindingPoints::PER_VIEW),
                      *SamplerBlockGenerator::GenerateSamplerBlock(BindingPoints::PER_VIEW, module_key));
  cg.GenerateSamplers(material_info.sampler_binding_map.GetBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE),
                      material_info.sampler_block);

  // 生成内置模块内容
  cg.AppendCode(ShaderCache::GetModuleContent(module_key));
  // 生成material的shader
  cg.AppendCode(ShaderCache::GetDataFromFile(material_info.frag_shader_file));

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