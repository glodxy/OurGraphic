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
}

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
  reader_.parse(data, data + size, root_);
  name_ = root_.get("name", "null").asString();
  if (root_.hasComment("params")) {
    params_ = root_.get("params");
    if (params_.hasComment("samplers")) {
      samplers_ = params_.get("samplers");
    }
    if (params_.hasComment("uniforms")) {
      uniforms_ = params_.get("uniforms");
    }
  }
  if (root_.hasComment("shaders")) {
    shaders_ = root_.get("shaders");
  }
}

bool MaterialParser::GetName(std::string &value) const noexcept {
  value = name_;
  return true;
}

bool MaterialParser::GetBlendingModel(BlendingMode &value) const noexcept {
  std::string type = root_.get("blending_mode", "OPAQUE").asString();
  if (kBlendingModeMap.find(type) == kBlendingModeMap.end()) {
    LOG_ERROR("MaterialParser", "blending_mode[{}] error!", type);
    value = BlendingMode::OPAQUE;
    return false;
  }
  value = kBlendingModeMap[type];
  return true;
}

bool MaterialParser::GetColorWrite(bool &value) const noexcept {
  value = root_.get("color_write", Json::Value(true)).asBool();
  return true;
}

bool MaterialParser::GetCullingMode(CullingMode &culling_mode) const noexcept {
  std::string type = root_.get("culling_mode", "NONE").asString();
  if (kCullingModeMap.find(type) == kCullingModeMap.end()) {
    LOG_ERROR("MaterialParser", "culling_mode[{}] error!", type);
    value = CullingMode::NONE;
    return false;
  }
  value = kCullingModeMap[type];
  return true;
}

bool MaterialParser::GetCustomDepthShaderSet(bool &value) const noexcept {
  value = root_.get("use_custom_depth_shader", false).asBool();
  return true;
}

bool MaterialParser::GetDepthTest(bool &value) const noexcept {
  value = root_.get("depth_test", false).asBool();
  return true;
}

bool MaterialParser::GetDepthWrite(bool &value) const noexcept {
  value = root_.get("depth_write", false).asBool();
  return true;
}

bool MaterialParser::GetDoubleSided(bool &value) const noexcept {
  value = root_.get("double_sided", false).asBool();
  return true;
}

bool MaterialParser::GetMaskThreshold(float &value) const noexcept {
  value = root_.get("mask_threshold", 1.0f).asFloat();
  return true;
}

bool MaterialParser::GetMaterialDomain(MaterialDomain &value) const noexcept {
  std::string type = root_.get("material_domain", "NONE").asString();
  if (kMaterialDomainMap.find(type) == kMaterialDomainMap.end()) {
    LOG_ERROR("MaterialParser", "material_domain[{}] error!", type);
    value = MaterialDomain::SURFACE;
    return false;
  }
  value = kCullingModeMap[type];
  return true;
}

bool MaterialParser::GetRefractionMode(RefractionMode &value) const noexcept {
  // todo:暂不支持折射
  value = RefractionMode::NONE;
  return true;
}

bool MaterialParser::GetRefractionType(RefractionType &value) const noexcept {
  // todo 暂不支持折射
  value = RefractionType::SOLID;
  return true;
}

bool MaterialParser::GetRequiredAttributes(AttributeBitset &value) const noexcept {
  uint32_t num = root_.get("required_attributes", 0).asUInt();
  value = num;
  return true;
}

bool MaterialParser::GetSamplerBlock(SamplerBlock &value) const noexcept {
  if (samplers_.isNull() || samplers_.empty()) {
    LOG_WARN("MaterialParser", "sampler not exist!");
    return false;
  }
  return ParseSamplers(value);
}

bool MaterialParser::GetUniformBlock(UniformBlock &value) const noexcept {
  if (uniforms_.isNull() || uniforms_.empty()) {
    LOG_WARN("MaterialParser", "uniform not exist!");
    return false;
  }
  return ParseUniforms(value);
}

bool MaterialParser::GetShadingModel(ShadingModel &value) const noexcept {
  std::string type = root_.get("shading_model", "LIT").asString();
  if (kShadingModelMap.find(type) == kShadingModelMap.end()) {
    LOG_ERROR("MaterialParser", "shading_model[{}] error!", type);
    value = ShadingModel::LIT;
    return false;
  }
  value = kShadingModelMap[type];
  return true;
}

bool MaterialParser::GetShader(ShaderBuilder &builder, uint8_t variant_key, ShaderType type) {
  if (shaders_.isNull() || shaders_.empty()) {
    LOG_ERROR("MaterialParser", "material[{}] cannot get node shaders",
              name_);
    return false;
  }
  uint8_t idx = 1;
  for(int i = 0; i < 8; ++i) {
    if ((variant_key & idx) != 0) {
      std::string text = ShaderCache::GetData(idx);
      builder.AppendData(text.data(), text.size());
    }
    idx <<= 1;
  }
  std::string shader_key = shaders_.get(kShaderTypeKeyMap[type], "").asString();
  if (shader_key.empty()) {
    LOG_ERROR("MaterialParser", "material[{}] cannot get shader:{}, variant:{}",
              name_, type, variant_key);
    return false;
  }
  std::string text = ShaderCache::GetShaderText(shader_key, uniform_block_, sampler_block_);
  builder.AppendData(text.data(), text.size());
  return true;
}

}