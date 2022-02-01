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
#include "shaderc/shaderc.hpp"
#include <map>
#include <string_view>
#include <fstream>
namespace our_graph {
static const std::map<std::string, UniformType> kUniformTypeMap = {
    {"bool", UniformType::BOOL}, {"bool2", UniformType::BOOL2},
    {"bool3", UniformType::BOOL3}, {"bool4", UniformType::BOOL4},
    {"float", UniformType::FLOAT}, {"float2", UniformType::FLOAT2},
    {"float3", UniformType::FLOAT3}, {"float4", UniformType::FLOAT4},
    {"int", UniformType::INT}, {"int2", UniformType::INT2},
    {"int3", UniformType::INT3}, {"int4", UniformType::INT4},
    {"uint", UniformType::UINT}, {"uint2", UniformType::UINT2},
    {"uint3", UniformType::UINT3}, {"uint4", UniformType::UINT4},
    {"mat3", UniformType::MAT3}, {"mat4", UniformType::MAT4}
};

static const std::map<std::string, SamplerType> kSamplerTypeMap = {
    {"sampler2D", SamplerType::SAMPLER_2D},
    {"sampler2DArray", SamplerType::SAMPLER_2D_ARRAY},
    {"sampler3D", SamplerType::SAMPLER_3D},
    {"samplerCubeMap", SamplerType::SAMPLER_CUBEMAP}
};

static const std::map<std::string, SamplerFormat> kSamplerFormatType = {
    {"int", SamplerFormat::INT}, {"uint", SamplerFormat::UINT},
    {"float", SamplerFormat::FLOAT}, {"shadow", SamplerFormat::SHADOW}
};

// 所有的property
static const std::map<std::string, std::tuple<size_t, UniformType, MaterialProperty::Property>> kProperties = {
    {"baseColor", {1, UniformType::FLOAT4, MaterialProperty::Property::BASE_COLOR}},
    {"roughness", {1, UniformType::FLOAT, MaterialProperty::Property::ROUGHNESS}}, // 粗糙度
    {"metallic", {1, UniformType::FLOAT, MaterialProperty::Property::METALLIC}}, // 金属度
    {"reflectance", {1, UniformType::FLOAT, MaterialProperty::Property::REFLECTANCE}}, // 反射度
    {"emissive", {1, UniformType::FLOAT4, MaterialProperty::Property::EMISSIVE}}, // 自发光
    {"normal", {1, UniformType::FLOAT3, MaterialProperty::Property::NORMAL}} // 法线
};

struct ParamInfo {
  std::string name;
  size_t size;
  UniformType uniform_type;
  SamplerType sampler_type;
  SamplerFormat format;

  enum {
    kInvalid,
    kUniform,
    kSampler
  } param_type;

  bool IsSampler() const {return param_type == kSampler;}
  bool IsUniform() const {return param_type == kUniform;}
};

static bool ParseType(const std::string& type, ParamInfo& info) {
  // 检查是否是数组
  size_t size = 1;
  size_t type_end = type.size() - 1;
  if (type.find('[') != std::string::npos) {
    size_t begin = type.find('[');
    type_end = begin;
    size_t end = begin + 1 ;
    while(end < type.size() && type[end] != ']') {++ end;}
    if (end >= type.size()) {
      LOG_ERROR("ParseType", "{} format error!", type);
      assert(false);
      return false;
    }
    std::string str_size = type.substr(begin + 1, end - begin);
    size = std::stoul(str_size);
    if (size <= 1) {
      LOG_ERROR("ParseType", "{}[{}] size format error!", type, str_size);
      assert(false);
      return false;
    }
  }

  // 提取type
  std::string type_name = type.substr(0, type_end);
  // 是uniform
  if (kUniformTypeMap.find(type) != kUniformTypeMap.end()) {
    info.uniform_type = kUniformTypeMap.at(type);
    info.param_type = ParamInfo::kUniform;
  } else if (kSamplerTypeMap.find(type) != kSamplerTypeMap.end()) {
    // 是sampler
    info.sampler_type = kSamplerTypeMap.at(type);
    info.param_type = ParamInfo::kSampler;
  } else {
    LOG_ERROR("ParseType", "Invalid Type:{}", type_name);
    info.param_type = ParamInfo::kInvalid;
    return false;
  }
  info.size = size;
  return true;
}

static bool ParseParameter(Json::Value node, ParamInfo& info) {
  std::string type = node.get("type", "").asString();
  std::string name = node.get("name", "").asString();
  if (name.empty()) {
    LOG_ERROR("ParseParameter", "Name Empty!");
    return false;
  }
  if (type.empty()) {
    LOG_ERROR("ParseParameter", "{}'s type empty!", name);
    return false;
  }
  info.name = name;
  if (!ParseType(type, info)) {
    LOG_ERROR("ParseParameter", "Parse {}'s type failed!", name);
    return false;
  }

  // 如果是sampler需提取format
  if (info.IsSampler()) {
    std::string format = node.get("format", "float").asString();
    if (kSamplerFormatType.find(format) == kSamplerFormatType.end()) {
      LOG_ERROR("ParseParameter", "param {} format {} error", name, format);
      return false;
    }
    info.format = kSamplerFormatType.at(format);
  }

  return true;
}

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


static const std::map<std::string, uint32_t> kVertexAttributeMap = {
    {"POSITION", 1 << POSITION},
    {"TANGENTS", 1 << TANGENTS}
};

static const std::map<std::string, uint32_t> kModuleKeyMap = {
    {"DEFERRED_LIGHT", ShaderVariantBit::DEFERRED_LIGHT},
    {"DIRECTIONAL_LIGHT", ShaderVariantBit::DIRECTIONAL_LIGHTING},
    {"DYNAMIC_LIGHTING", ShaderVariantBit::DYNAMIC_LIGHTING}
};

static const std::map<std::string, RenderPath> kRenderPathMap = {
    {"DEFERRED", RenderPath::DEFERRED},
    {"FORWARD", RenderPath::FORWARD}
};


MaterialParser::MaterialParser(
    Backend backend,
    const void *data,
    size_t size) {
  reader_.parse(reinterpret_cast<const char*>(data) , reinterpret_cast<const char*>(data) + size, root_);
}

bool MaterialParser::Parse() {
  material_info_.refraction_type = RefractionType::SOLID;
  material_info_.refraction_mode = RefractionMode::NONE;
  ParseName();
  ParseVersion();
  ParseRenderPath();
  ParseRefractionType();
  ParseRefractionMode();
  ParseRequiredAttributes();
  ParseMaterialDomain();
  ParseMaskThreshold();
  ParseDoubleSided();
  ParseDepthWrite();
  ParseDepthTest();
  ParseCustomDepthShaderSet();
  ParseCullingMode();
  ParseColorWrite();
  ParseBlendingModel();
  ParseVariables();
  ParseShadingModel();
  ParseParams();
  ParseSubpass();
  GenerateShaderText();
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
    while((end = attributes.find('|', begin)) != std::string::npos) {
      std::string sub_attr = attributes.substr(begin, end - begin);
      if (kVertexAttributeMap.find(sub_attr) != kVertexAttributeMap.end()) {
        num |= kVertexAttributeMap.at(sub_attr);
      }
      begin = end + 1;
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


bool MaterialParser::GetSamplerBlock(SamplerBlock &value) const noexcept {
  value = material_info_.sampler_block;
  return true;
}

bool MaterialParser::GetSamplerBindingMap(SamplerBindingMap &map) const noexcept {
  map = material_info_.sampler_binding_map;
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

void MaterialParser::ParseRenderPath() noexcept {
  RenderPath value;
  std::string type = root_.get("render_path", "DEFERRED").asString();
  if (kRenderPathMap.find(type) == kRenderPathMap.end()) {
    LOG_ERROR("MaterialParser", "render_path[{}] error!", type);
    value = RenderPath::DEFERRED;
  } else {
    value = kRenderPathMap.at(type);
  }
  material_info_.render_path = value;
}
bool MaterialParser::GetRenderPath(RenderPath &value) const noexcept {
  value = material_info_.render_path;
  return true;
}



uint32_t MaterialParser::InterParseModuleKey() noexcept {
  std::string modules = root_.get("modules", "").asString();
  uint32_t num = 0;
  if (!modules.empty()) {
    size_t end = 0;
    uint32_t begin = 0;
    while((end = modules.find('|', begin)) != std::string::npos) {
      std::string sub_attr = modules.substr(begin, end - begin);
      if (kModuleKeyMap.find(sub_attr) != kModuleKeyMap.end()) {
        num |= kModuleKeyMap.at(sub_attr);
      }
      begin = end + 1;
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
    size_t idx = 0;
    for (const auto& v: v_root) {
      if (idx >= MATERIAL_VARIABLES_COUNT) {
        break;
      }
      if (v.isNull() || v.empty()) {
        continue;
      }
      Variant tmp;
      tmp.name = v.get("name", "").asString();
      tmp.type = v.get("type", "vec3").asString();
      tmp.size = v.get("size", 1).asUInt();
      material_info_.variant_list[idx++] = tmp;
    }
  }
}

void MaterialParser::ParseSubpass() noexcept {
  auto subpass = root_.get("subpass", Json::ValueType::nullValue);
  if (subpass.isNull() || !subpass.isArray()) {
    LOG_ERROR("ParseSubpass", "{} has no subpass node",
              name_);
    return;
  }
  InterParseModuleKey();
  for (const auto& info : subpass) {
    SubpassInfo subpass_info;
    ParseShader(subpass_info, info);

    material_info_.pass_list.push_back(subpass_info);
  }
}

void MaterialParser::ParseShader(SubpassInfo& info, Json::Value node) noexcept {
  // 得到使用的模块
  uint32_t module_key = module_key_;
  // 得到vertex shader file以及frag shader file
  info.vertex_shader = "default_vert.vert";
  info.frag_shader = "default_frag.frag";
  auto shaders = node.get("shaders", Json::ValueType::nullValue);
  if (!shaders.isNull()) {
    info.vertex_shader = shaders.get("vertex", "default_vertex.vert").asString();
    info.frag_shader = shaders.get("frag", "default_frag.frag").asString();
  }
}

void MaterialParser::GenerateShaderText() {
  // 遍历所有subpass
  for (size_t i = 0; i < material_info_.pass_list.size(); ++i) {
    std::string vertex = InterGenerateShader(ShaderType::VERTEX, module_key_, i);
    std::string frag = InterGenerateShader(ShaderType::FRAGMENT, module_key_, i);
    shader_text_.push_back({vertex, frag});
  }
}

std::string MaterialParser::InterGenerateShader(ShaderType type, uint32_t module_key, uint8_t subpass_idx) noexcept {
  static ShaderGenerator sg;
  return sg.CreateShaderText(type, material_info_, module_key, subpass_idx);
}



bool MaterialParser::GetShader(ShaderBuilder &builder, ShaderType type, uint8_t subpass_idx) {
  switch (type) {
    case ShaderType::VERTEX: {
      auto data = ShaderCache::CompileFile(material_info_.pass_list[subpass_idx].vertex_shader,
                              shaderc_glsl_vertex_shader, shader_text_[subpass_idx].first);
      builder.AppendData(data.data(), data.size() * 4);
      break;
    }
    case ShaderType::FRAGMENT: {
      auto data = ShaderCache::CompileFile(material_info_.pass_list[subpass_idx].frag_shader,
                              shaderc_glsl_fragment_shader, shader_text_[subpass_idx].second);
      builder.AppendData(data.data(), data.size() * 4);
      break;
    }
    default:
      return false;
  }

  return true;
}

uint32_t MaterialParser::GetModuleKey() const noexcept {
  return module_key_;
}

bool MaterialParser::IsProperty(const std::string &name,
                                size_t size,
                                UniformType type,
                                MaterialProperty::Property &prop) {
  auto iter = kProperties.find(name);
  if (iter == kProperties.end()) {
    return false;
  }
  if (size != std::get<0>(iter->second) || type != std::get<1>(iter->second)) {
    return false;
  }
  prop = std::get<2>(iter->second);
  return true;
}

// 根据params生成Block    +
bool MaterialParser::ParseParams() noexcept {
  auto param_root = root_.get("params", Json::ValueType::nullValue);
  if (param_root.empty() || param_root.isNull() || !param_root.isArray()) {
    return false;
  }
  SamplerBlock::Builder sbb;
  UniformBlock::Builder ubb;
  for (const auto& param_node : param_root) {
    ParamInfo info;
    if (!ParseParameter(param_node, info)) {
      continue;
    }
    if (info.IsSampler()) {
      sbb.Add(info.name, info.sampler_type, info.format);
    } else if (info.IsUniform()) {
      ubb.Add(info.name, info.size, info.uniform_type);
      // 是内置变量则标记
      MaterialProperty::Property prop;
      if (IsProperty(info.name, info.size, info.uniform_type, prop)) {
        material_info_.property_list.push_back(prop);
      }
    }
  }

  if (material_info_.blending_mode == BlendingMode::MASKED) {
    ubb.Add("_maskThreshold", 1, UniformType::FLOAT);
  }

  if (material_info_.has_double_sided_capability) {
    ubb.Add("_doubleSided", 1, UniformType::BOOL);
  }
  // 一定要启用position
  material_info_.required_attributes.set(VertexAttribute::POSITION);

  material_info_.sampler_block = sbb.Name("MaterialParams").Build();
  material_info_.uniform_block = ubb.Name("MaterialParams").Build();

  material_info_.sampler_binding_map.Init(&material_info_.sampler_block);
  return true;
}

}