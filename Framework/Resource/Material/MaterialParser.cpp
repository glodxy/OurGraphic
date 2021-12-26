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
#include <map>
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
}

void MaterialParser::ParseName() const noexcept {
  name_ = root_.get("name", "null").asString();
}

bool MaterialParser::GetName(std::string &value) const noexcept {
  value = name_;
  return true;
}

void MaterialParser::ParseBlendingModel() const noexcept {
  BlendingMode value;
  std::string type = root_.get("blending_mode", "OPAQUE").asString();
  if (kBlendingModeMap.find(type) == kBlendingModeMap.end()) {
    LOG_ERROR("MaterialParser", "blending_mode[{}] error!", type);
    value = BlendingMode::OPAQUE;
    return false;
  }
  value = kBlendingModeMap.at(type);
  material_info_.blending_mode = value;
}

bool MaterialParser::GetBlendingModel(BlendingMode &value) const noexcept {
  value = material_info_.blending_mode;
  return true;
}

void MaterialParser::ParseColorWrite() const noexcept {
  material_info_.color_write_ = root_.get("color_write", Json::Value(true)).asBool();
}

bool MaterialParser::GetColorWrite(bool &value) const noexcept {
  value = material_info_.color_write;
  return true;
}

void MaterialParser::ParseCullingMode() const noexcept {
  CullingMode culling_mode;
  std::string type = root_.get("culling_mode", "NONE").asString();
  if (kCullingModeMap.find(type) == kCullingModeMap.end()) {
    LOG_ERROR("MaterialParser", "culling_mode[{}] error!", type);
    culling_mode = CullingMode::NONE;
    return false;
  }
  culling_mode = kCullingModeMap.at(type);
  material_info_.culling_mode = culling_mode;
}

bool MaterialParser::GetCullingMode(CullingMode &culling_mode) const noexcept {
  culling_mode = material_info_.culling_mode;
  return true;
}

void MaterialParser::ParseCustomDepthShaderSet() const noexcept {
  material_info_.has_custom_depth_shader = root_.get("use_custom_depth_shader", false).asBool();
}
bool MaterialParser::GetCustomDepthShaderSet(bool &value) const noexcept {
  value = material_info_.has_custom_depth_shader;
  return true;
}

void MaterialParser::ParseDepthTest() const noexcept {
  material_info_.enable_depth_test = root_.get("depth_test", false).asBool();
}
bool MaterialParser::GetDepthTest(bool &value) const noexcept {
  value = material_info_.enable_depth_test;
  return true;
}

void MaterialParser::ParseDepthWrite() const noexcept {
  material_info_.depth_write = root_.get("depth_write", false).asBool();
}
bool MaterialParser::GetDepthWrite(bool &value) const noexcept {
  value = material_info_.depth_write;
  return true;
}

void MaterialParser::ParseDoubleSided() const noexcept {
  material_info_.has_double_sided_capability = root_.get("double_sided", false).asBool();
}
bool MaterialParser::GetDoubleSided(bool &value) const noexcept {
  value = material_info_.has_double_sided_capability;
  return true;
}

void MaterialParser::ParseMaskThreshold() const noexcept {
  material_info_.mask_threshold = root_.get("mask_threshold", 1.0f).asFloat();
}
bool MaterialParser::GetMaskThreshold(float &value) const noexcept {
  value = material_info_.mask_threshold;
  return true;
}

void MaterialParser::ParseMaterialDomain() const noexcept {
  MaterialDomain value;
  std::string type = root_.get("material_domain", "NONE").asString();
  if (kMaterialDomainMap.find(type) == kMaterialDomainMap.end()) {
    LOG_ERROR("MaterialParser", "material_domain[{}] error!", type);
    value = MaterialDomain::SURFACE;
    return false;
  }
  value = kMaterialDomainMap.at(type);
  material_info_.domain = value;
}
bool MaterialParser::GetMaterialDomain(MaterialDomain &value) const noexcept {
  value = material_info_.domain;
  return true;
}

void MaterialParser::ParseRefractionMode() const noexcept {
  // todo:暂不支持折射
  material_info_.refraction_mode = RefractionMode::NONE;
}
bool MaterialParser::GetRefractionMode(RefractionMode &value) const noexcept {
  value = material_info_.refraction_mode;
  return true;
}

void MaterialParser::ParseRefractionType() const noexcept {
  // todo:暂不支持折射
  material_info_.refraction_type = RefractionType::SOLID;
}
bool MaterialParser::GetRefractionType(RefractionType &value) const noexcept {
  value = material_info_.refraction_type;
  return true;
}

void MaterialParser::ParseRequiredAttributes() const noexcept {
  uint32_t num = root_.get("required_attributes", 0).asUInt();
  material_info_.required_attributes = num;
}
bool MaterialParser::GetRequiredAttributes(AttributeBitset &value) const noexcept {
  value = material_info_.required_attributes;
  return true;
}

bool MaterialParser::ParseSamplers(SamplerBlock &sampler_block) const {
  if (samplers_.isNull() || samplers_.empty()) {
    LOG_WARN("MaterialParser", "sampler not exist!");
    return false;
  }
}
bool MaterialParser::GetSamplerBlock(SamplerBlock &value) const noexcept {
  value = material_info_.sampler_block;
}

bool MaterialParser::ParseUniforms() const {
  if (uniforms_.isNull() || uniforms_.empty()) {
    LOG_WARN("MaterialParser", "uniform not exist!");
    return false;
  }
}
bool MaterialParser::GetUniformBlock(UniformBlock &value) const noexcept {
  value = material_info_.uniform_block;
}

bool MaterialParser::GetShadingModel(ShadingModel &value) const noexcept {
  std::string type = root_.get("shading_model", "LIT").asString();
  if (kShadingModelMap.find(type) == kShadingModelMap.end()) {
    LOG_ERROR("MaterialParser", "shading_model[{}] error!", type);
    value = ShadingModel::LIT;
    return false;
  }
  value = kShadingModelMap.at(type);
  return true;
}

bool MaterialParser::GetShader(ShaderBuilder &builder, uint8_t variant_key, ShaderType type) {
  if (shaders_.isNull() || shaders_.empty()) {
    LOG_ERROR("MaterialParser", "material[{}] cannot get node shaders",
              name_);
    return false;
  }
  // 1. 先构建material自身所定义的shader
  if (kShaderTypeKeyMap.find(type) == kShaderTypeKeyMap.end()) {
    LOG_ERROR("MaterialParser", "material[{}] cannot find shader type:{}",
              name_, type);
    return false;
  }
  std::string shader_key = shaders_.get(kShaderTypeKeyMap.at(type), "").asString();
  if (shader_key.empty()) {
    LOG_ERROR("MaterialParser", "material[{}] cannot get shader:{}, variant:{}",
              name_, type, variant_key);
    return false;
  }
  std::string text = ShaderCache::GetShaderText(shader_key, uniform_block_, sampler_block_);
  builder.AppendData(text.data(), text.size());

  // 2. 构建库中的shader
  uint8_t idx = 1;
  for(int i = 0; i < 8; ++i) {
    if ((variant_key & idx) != 0) {
      std::string text = ShaderCache::GetData(idx);
      builder.AppendData(text.data(), text.size());
    }
    idx <<= 1;
  }

  return true;
}

}