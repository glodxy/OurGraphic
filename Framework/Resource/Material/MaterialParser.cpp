//
// Created by Glodxy on 2021/12/5.
//

#include "MaterialParser.h"
#include "json/json.h"
#include "Framework/Resource/Material/SamplerBlock.h"
#include "Framework/Resource/Material/UniformBlock.h"
#include "Framework/Resource/Material/ShaderBuilder.h"
#include "include/GlobalEnum.h"
#include "ShaderCache.h"
#include "Framework/Resource/Material/Shader/ShaderGenerator.h"
#include <map>
#include <string_view>
namespace our_graph {
static const std::map<std::string, BlendingMode> kBlendingModeMap = {
    {"OPAQUE", BlendingMode::OPAQUE},
    {"TRANSPARENT", BlendingMode::TRANSPARENT},
    {"ADD", BlendingMode::ADD},
    {"FADE", BlendingMode::FADE},
    {"MASKED", BlendingMode::MASKED},
    {"MULTIPLY", BlendingMode::MULTIPLY},
    {"SCREEN", BlendingMode::SCREEN}
};

static const std::map<std::string, CullingMode> kCullingModeMap = {
    {"NONE", CullingMode::NONE},
    {"FRONT", CullingMode::FRONT},
    {"BACK", CullingMode::BACK},
    {"ALL", CullingMode::FRONT_AND_BACK},
};

static const std::map<std::string, MaterialDomain> kMaterialDomainMap = {
    {"SURFACE", MaterialDomain::SURFACE},
    {"POST_PROCESS", MaterialDomain::POST_PROCESS}
};

static const std::map<std::string, ShadingModel> kShadingModelMap = {
    {"UNLIT", ShadingModel::UNLIT},
    {"LIT", ShadingModel::LIT},
    {"SUBSURFACE", ShadingModel::SUBSURFACE},
    {"CLOTH", ShadingModel::CLOTH},
    {"SPECULAR_GLOSSINESS", ShadingModel::SPECULAR_GLOSSINESS}
};

static const std::map<MaterialParser::ShaderType, std::string> kShaderTypeKeyMap = {
    {MaterialParser::ShaderType::VERTEX, "vertex"},
    {MaterialParser::ShaderType::FRAGMENT, "frag"}
};

static const std::map<std::string, uint32_t> kVertexAttributeMap = {
    {"POSITION", 1 << POSITION},
};

static const std::map<std::string, uint32_t> kModuleKeyMap = {
    {"DIRECTIONAL_LIGHT", ShaderVariantBit::DIRECTIONAL_LIGHTING},
    {"DEPTH", ShaderVariantBit::DEPTH}
};


MaterialParser::MaterialParser(
    Backend backend,
    const void *data,
    size_t size) {
  reader_.parse(reinterpret_cast<const char*>(data) , reinterpret_cast<const char*>(data) + size, root_);
  params_ = root_.get("params", Json::ValueType::nullValue);
  if (!(params_.isNull() || params_.empty())) {
    samplers_ = params_.get("samplers", Json::ValueType::nullValue);
    uniforms_ = params_.get("uniforms", Json::ValueType::nullValue);
  }
  shaders_ = root_.get("shaders", Json::ValueType::nullValue);
}

bool MaterialParser::Parse() {
  material_info_.refraction_type = RefractionType::SOLID;
  material_info_.refraction_mode = RefractionMode::NONE;
  return true;
}


void MaterialParser::ParseVersion() noexcept {
  version_ = root_.get("version", 1).asUInt();
}
bool MaterialParser::GetVersion(uint32_t &value) const noexcept {
  value = version_;
  return true;
}

void MaterialParser::ParseName() noexcept {
  name_ = root_.get("name", "null").asString();
}

bool MaterialParser::GetName(std::string &value) const noexcept {
  value = name_;
  return true;
}

void MaterialParser::ParseBlendingModel() noexcept {
  BlendingMode value;
  std::string type = root_.get("blending_mode", "OPAQUE").asString();
  if (kBlendingModeMap.find(type) == kBlendingModeMap.end()) {
    LOG_ERROR("MaterialParser", "blending_mode[{}] error!", type);
    value = BlendingMode::OPAQUE;
  } else {
    value = kBlendingModeMap.at(type);
  }
  material_info_.blending_mode = value;
}

bool MaterialParser::GetBlendingModel(BlendingMode &value) const noexcept {
  value = material_info_.blending_mode;
  return true;
}

void MaterialParser::ParseColorWrite() noexcept {
  material_info_.color_write = root_.get("color_write", Json::Value(true)).asBool();
}

bool MaterialParser::GetColorWrite(bool &value) const noexcept {
  value = material_info_.color_write;
  return true;
}

void MaterialParser::ParseCullingMode() noexcept {
  CullingMode culling_mode;
  std::string type = root_.get("culling_mode", "NONE").asString();
  if (kCullingModeMap.find(type) == kCullingModeMap.end()) {
    LOG_ERROR("MaterialParser", "culling_mode[{}] error!", type);
    culling_mode = CullingMode::NONE;
  } else {
    culling_mode = kCullingModeMap.at(type);
  }
  material_info_.culling_mode = culling_mode;
}

bool MaterialParser::GetCullingMode(CullingMode &culling_mode) const noexcept {
  culling_mode = material_info_.culling_mode;
  return true;
}

void MaterialParser::ParseCustomDepthShaderSet() noexcept {
  material_info_.has_custom_depth_shader = root_.get("use_custom_depth_shader", false).asBool();
}
bool MaterialParser::GetCustomDepthShaderSet(bool &value) const noexcept {
  value = material_info_.has_custom_depth_shader;
  return true;
}

void MaterialParser::ParseDepthTest() noexcept {
  material_info_.enable_depth_test = root_.get("depth_test", false).asBool();
}
bool MaterialParser::GetDepthTest(bool &value) const noexcept {
  value = material_info_.enable_depth_test;
  return true;
}

void MaterialParser::ParseDepthWrite() noexcept {
  material_info_.depth_write = root_.get("depth_write", false).asBool();
}
bool MaterialParser::GetDepthWrite(bool &value) const noexcept {
  value = material_info_.depth_write;
  return true;
}

void MaterialParser::ParseDoubleSided() noexcept {
  material_info_.has_double_sided_capability = root_.get("double_sided", false).asBool();
}
bool MaterialParser::GetDoubleSided(bool &value) const noexcept {
  value = material_info_.has_double_sided_capability;
  return true;
}

void MaterialParser::ParseMaskThreshold() noexcept {
  material_info_.mask_threshold = root_.get("mask_threshold", 1.0f).asFloat();
}
bool MaterialParser::GetMaskThreshold(float &value) const noexcept {
  value = material_info_.mask_threshold;
  return true;
}

void MaterialParser::ParseMaterialDomain() noexcept {
  MaterialDomain value;
  std::string type = root_.get("material_domain", "NONE").asString();
  if (kMaterialDomainMap.find(type) == kMaterialDomainMap.end()) {
    LOG_ERROR("MaterialParser", "material_domain[{}] error!", type);
    value = MaterialDomain::SURFACE;
  } else {
    value = kMaterialDomainMap.at(type);
  }
  material_info_.domain = value;
}
bool MaterialParser::GetMaterialDomain(MaterialDomain &value) const noexcept {
  value = material_info_.domain;
  return true;
}

void MaterialParser::ParseRefractionMode() noexcept {
  // todo:暂不支持折射
  material_info_.refraction_mode = RefractionMode::NONE;
}
bool MaterialParser::GetRefractionMode(RefractionMode &value) const noexcept {
  value = material_info_.refraction_mode;
  return true;
}

void MaterialParser::ParseRefractionType() noexcept {
  // todo:暂不支持折射
  material_info_.refraction_type = RefractionType::SOLID;
}
bool MaterialParser::GetRefractionType(RefractionType &value) const noexcept {
  value = material_info_.refraction_type;
  return true;
}

void MaterialParser::ParseRequiredAttributes() noexcept {
  std::string attributes = root_.get("required_attributes", "").asString();
  uint32_t num = 0;
  if (!attributes.empty()) {
    size_t end = 0;
    uint32_t begin = 0;
    while((end = attributes.find('|', end)) != std::string::npos) {
      std::string sub_attr = attributes.substr(begin, end - begin + 1);
      if (kVertexAttributeMap.find(sub_attr) != kVertexAttributeMap.end()) {
        num |= kVertexAttributeMap.at(sub_attr);
      }
    }
    if (end != begin) {
      std::string sub_attr = attributes.substr(begin);
      if (kVertexAttributeMap.find(sub_attr) != kVertexAttributeMap.end()) {
        num |= kVertexAttributeMap.at(sub_attr);
      }
    }
  }

  material_info_.required_attributes = num;
}
bool MaterialParser::GetRequiredAttributes(AttributeBitset &value) const noexcept {
  value = material_info_.required_attributes;
  return true;
}

bool MaterialParser::ParseSamplers() {
  if (samplers_.isNull() || samplers_.empty()) {
    LOG_WARN("MaterialParser", "sampler not exist!");
    return false;
  }

  material_info_.sampler_binding_map.Init(&material_info_.sampler_block);
  return true;
}
bool MaterialParser::GetSamplerBlock(SamplerBlock &value) const noexcept {
  value = material_info_.sampler_block;
  return true;
}

bool MaterialParser::GetSamplerBindingMap(SamplerBindingMap &map) const noexcept {
  map = material_info_.sampler_binding_map;
  return true;
}

bool MaterialParser::ParseUniforms() {
  if (uniforms_.isNull() || uniforms_.empty()) {
    LOG_WARN("MaterialParser", "uniform not exist!");
    return false;
  }
  return true;
}
bool MaterialParser::GetUniformBlock(UniformBlock &value) const noexcept {
  value = material_info_.uniform_block;
  return true;
}

void MaterialParser::ParseShadingModel() noexcept {
  ShadingModel value;
  std::string type = root_.get("shading_model", "LIT").asString();
  if (kShadingModelMap.find(type) == kShadingModelMap.end()) {
    LOG_ERROR("MaterialParser", "shading_model[{}] error!", type);
    value = ShadingModel::LIT;
  } else {
    value = kShadingModelMap.at(type);
  }
  material_info_.shading_model = value;
}
bool MaterialParser::GetShadingModel(ShadingModel &value) const noexcept {
  value = material_info_.shading_model;
  return true;
}


uint32_t MaterialParser::InterParseModuleKey() noexcept {
  std::string modules = root_.get("modules", "").asString();
  uint32_t num = 0;
  if (!modules.empty()) {
    size_t end = 0;
    uint32_t begin = 0;
    while((end = modules.find('|', end)) != std::string::npos) {
      std::string sub_attr = modules.substr(begin, end - begin + 1);
      if (kModuleKeyMap.find(sub_attr) != kModuleKeyMap.end()) {
        num |= kModuleKeyMap.at(sub_attr);
      }
    }
    if (end != begin) {
      std::string sub_attr = modules.substr(begin);
      if (kModuleKeyMap.find(sub_attr) != kModuleKeyMap.end()) {
        num |= kModuleKeyMap.at(sub_attr);
      }
    }
  }

  module_key_ = num;
  return module_key_;
}

void MaterialParser::ParseVariables() noexcept {
  Json::Value v_root = root_.get("variables", Json::ValueType::nullValue);

  if (v_root.isNull() || v_root.empty()) {
    return;
  }
  if (v_root.isArray()) {
    // 解析其中的列表
    for (const auto& v: v_root) {
      if (v.isNull() || v.empty()) {
        continue;
      }
      Variant tmp;
      tmp.name = v.get("name", "").asString();
      tmp.type = v.get("type", "vec3").asString();
      tmp.size = v.get("size", 1).asUInt();
      material_info_.variant_list.push_back(tmp);
    }
  }
}


void MaterialParser::ParseShader() noexcept {
  if (shaders_.isNull() || shaders_.empty()) {
    LOG_ERROR("MaterialParser", "material[{}] cannot get node shaders",
              name_);
    return;
  }
  // 得到使用的模块
  uint32_t module_key = InterParseModuleKey();
  // 顶点着色器
  vertex_shader_text_ = InterParseShader(ShaderType::VERTEX, module_key);
  // 片段着色器
  frag_shader_text_ = InterParseShader(ShaderType::FRAGMENT, module_key);

}

std::string MaterialParser::InterParseShader(ShaderType type, uint32_t module_key) noexcept {
  // 1. 先构建material自身所定义的shader
  if (kShaderTypeKeyMap.find(type) == kShaderTypeKeyMap.end()) {
    LOG_ERROR("MaterialParser", "material[{}] cannot find shader type:{}",
              name_, type);
    return "";
  }
  std::string shader_file = shaders_.get(kShaderTypeKeyMap.at(type), "").asString();
  if (shader_file.empty()) {
    LOG_ERROR("MaterialParser", "material[{}] cannot get shader:{}, modules:{}",
              name_, type, module_key);
    return "";
  }

  ShaderGenerator sg;
  return sg.CreateShaderText(type, material_info_, module_key);
}


bool MaterialParser::GetShader(ShaderBuilder &builder, ShaderType type) {
  switch (type) {
    case ShaderType::VERTEX:
      builder.AppendData(vertex_shader_text_.data(), vertex_shader_text_.size());
      break;
    case ShaderType::FRAGMENT:
      builder.AppendData(frag_shader_text_.data(), frag_shader_text_.size());
      break;
    default:
      return false;
  }

  return true;
}

uint32_t MaterialParser::GetModuleKey() const noexcept {
  return module_key_;
}

}