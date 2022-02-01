//
// Created by Glodxy on 2021/12/20.
//

#include "CodeGenerator.h"

namespace our_graph {

CodeGenerator::CodeGenerator(std::stringstream &ss, ShaderType shader_type) : ss_(ss) {
  shader_type_ = shader_type;
}

void CodeGenerator::GenerateSeparator() {
  ss_ << "\n";
}

void CodeGenerator::GenerateHead() {
  ss_ << "#version 450 core\n\n";
}

void CodeGenerator::GenerateDefine(const std::string &name, bool value) {
  if (value) {
    ss_ << "#define " << name << "\n";
  }
}

void CodeGenerator::GenerateDefine(const std::string &name, uint32_t value) {
  ss_ << "#define " << name << " " << value << "\n";
}

void CodeGenerator::GenerateDefine(const std::string &name, const std::string &value) {
  ss_ << "#define " << name << " " << value << "\n";
}

void CodeGenerator::GenerateIndexedDefine(const std::string &name, uint32_t index, uint32_t value) {
  ss_ << "#define" << name << index << value << "\n";
}

void CodeGenerator::AppendCode(const std::string &code) {
  GenerateSeparator();
  ss_ << code;
  GenerateSeparator();
}

void CodeGenerator::GenerateVariable(const std::string &name, const std::string &type, uint32_t size, size_t idx) {
  if (!name.empty() && !type.empty()) {
    if (shader_type_ == ShaderType::VERTEX) {
      ss_ << "\n#define VARIABLE_CUSTOM" << idx << " " << name << "\n";
      ss_ << "\n#define VARIABLE_CUSTOM_AT" << idx << " variable_" << name << "\n";
      ss_ << "layout(location=" << idx << ") out " << type << " variable_" << name;
      if (size > 1) {
        ss_ << "[" << size << "]";
      }
      ss_<< ";\n";
    } else {
      ss_ << "layout(location=" << idx << ") in " << type << " variable_" << name;
      if (size > 1) {
        ss_ << "[" << size << "]";
      }
      ss_<< ";\n";
    }
  }
}

void CodeGenerator::GenerateShaderInput(const AttributeBitset &attributes) {
  bool has_tangent = attributes.test(VertexAttribute::TANGENTS);
  GenerateDefine("HAS_ATTRIBUTE_TANGENTS", has_tangent);

  bool has_color = attributes.test(VertexAttribute::COLOR);
  GenerateDefine("HAS_ATTRIBUTE_COLOR", has_color);

  bool has_uv0 = attributes.test(VertexAttribute::UV0);
  GenerateDefine("HAS_ATTRIBUTE_UV0", has_uv0);

  bool has_uv1 = attributes.test(VertexAttribute::UV1);
  GenerateDefine("HAS_ATTRIBUTE_UV1", has_uv1);

  bool has_bone_indices = attributes.test(VertexAttribute::BONE_INDICES);
  GenerateDefine("HAS_ATTRIBUTE_BONE_INDICES", has_bone_indices);

  bool has_bone_weight = attributes.test(VertexAttribute::BONE_WEIGHTS);
  GenerateDefine("HAS_ATTRIBUTE_BONE_WEIGHTS", has_bone_weight);

  for (int i = 0; i < MAX_VERTEX_ATTRIBUTE_CUSTOM_COUNT; ++i) {
    bool has_custom_n = attributes.test(VertexAttribute::CUSTOM0 + i);
    if (has_custom_n) {
      GenerateIndexedDefine("HAS_ATTRIBUTE_CUSTOM_DATA", i, 1);
    }
  }

  // 对于vertex类型，需要生成资源的location
  if (shader_type_ == ShaderType::VERTEX) {
    ss_ << "\n";
    // position
    GenerateDefine("LOCATION_POSITION", uint32_t(VertexAttribute::POSITION));

    if (has_tangent) {
      GenerateDefine("LOCATION_TANGENTS", uint32_t(VertexAttribute::TANGENTS));
    }

    if (has_color) {
      GenerateDefine("LOCATION_COLOR", uint32_t(VertexAttribute::COLOR));
    }

    if (has_uv0) {
      GenerateDefine("LOCATION_UV0", uint32_t(VertexAttribute::UV0));
    }

    if (has_uv1) {
      GenerateDefine("LOCATION_UV1", uint32_t(VertexAttribute::UV1));
    }

    if (has_bone_indices) {
      GenerateDefine("LOCATION_BONE_INDICES", uint32_t(VertexAttribute::BONE_INDICES));
    }

    if (has_bone_weight) {
      GenerateDefine("LOCATION_BONE_WEIGHT", uint32_t(VertexAttribute::BONE_WEIGHTS));
    }
    // custom
    for (int i = 0; i < MAX_VERTEX_ATTRIBUTE_CUSTOM_COUNT; ++i) {
      bool has_custom_n = attributes.test(VertexAttribute::CUSTOM0 + i);
      if (has_custom_n) {
        GenerateIndexedDefine("LOCATION_CUSTOM_DATA", i, uint32_t(VertexAttribute::CUSTOM0) + i);
      }
    }
  }
}

void CodeGenerator::GenerateUniforms(uint32_t binding, const UniformBlock &uniform_block) {
  const auto& info_list = uniform_block.GetUniformInfoList();
  if (info_list.empty()) {
    return;
  }

  std::string uniform_name = uniform_block.GetName();
  std::string instance_name = uniform_name;
  instance_name.front() = std::tolower(instance_name[0]);

  // 正式写入字符串
  ss_ << "\n";
  ss_ << "layout(binding=" << binding << ", set=0) uniform " << uniform_name << "{\n";
  for (const auto& info : info_list) {
    const char* const type = GetUniformTypeName(info.type);
    ss_ << type << " " << info.name;
    if (info.size > 1) {
      ss_ << "[" << info.size << "]";
    }
    ss_ << ";\n";
  }
  ss_ << "} " << instance_name << ";\n";
}

void CodeGenerator::GenerateSubpass(std::string name, uint8_t attachment_idx, uint8_t binding) {
  std::string type_name = "subpassInput";
  ss_ << "layout(input_attachment_index = "<<attachment_idx
      << ", set = 2, binding = "<< binding << ")";
  ss_ << "uniform "<< type_name << " " << name;
  ss_ << ";\n";

  ss_ << "\n";
}

void CodeGenerator::GenerateMaterialProperty(MaterialProperty::Property property, bool set) {
  if (set) {
    ss_ << "#define " << "MATERIAL_HAS_" << GetMaterialPropertyName(property) << "\n";
  }
}

void CodeGenerator::GenerateMaterialProperties(std::vector<MaterialProperty::Property> props) {
  for (const auto& prop : props) {
    GenerateMaterialProperty(prop, true);
  }
}

void CodeGenerator::GenerateRenderPath(RenderPath path) {
  GenerateDefine(GetRenderPathDefine(path), true);
}

void CodeGenerator::GenerateSamplers(uint32_t binding, const SamplerBlock &sampler_block) {
  const auto& info_list = sampler_block.GetSamplerInfoList();
  if (info_list.empty()) {
    return;
  }

  for (const auto& info : info_list) {
    std::string uniform_name = SamplerBlock::GetUniformName(sampler_block.GetName(), info.name);
    auto type = info.type;
    const char* const type_name = GetSamplerTypeName(type, info.format, info.multi_sample);
    const uint32_t bind_idx = binding + info.offset;
    ss_ << "layout(binding=" << bind_idx << ", set=1)";
    ss_ << " uniform " << type_name << " " << uniform_name << ";\n";
  }
  ss_ << "\n";
}

const char * CodeGenerator::GetUniformTypeName(UniformBlock::Type type) {
  using Type = UniformBlock::Type;
  switch (type) {
    case Type::BOOL: return "bool";
    case Type::BOOL2: return "bvec2";
    case Type::BOOL3: return "bvec3";
    case Type::BOOL4: return "bvec4";

    case Type::FLOAT: return "float";
    case Type::FLOAT2: return "vec2";
    case Type::FLOAT3: return "vec3";
    case Type::FLOAT4: return "vec4";

    case Type::INT: return "int";
    case Type::INT2: return "ivec2";
    case Type::INT3: return "ivec3";
    case Type::INT4: return "ivec4";

    case Type::UINT: return "uint";
    case Type::UINT2: return "uvec2";
    case Type::UINT3: return "uvec3";
    case Type::UINT4: return "uvec4";

    case Type::MAT3: return "mat3";
    case Type::MAT4: return "mat4";
  }
}

const char * CodeGenerator::GetSamplerTypeName(SamplerType type, SamplerFormat format, bool multi_sample) {
  // todo:暂不支持multi sample
  assert(!multi_sample);
  switch (type) {
    case SamplerType::SAMPLER_2D: {
      switch (format) {
        case SamplerFormat::FLOAT: return "sampler2D";
        case SamplerFormat::INT: return "isampler2D";
        case SamplerFormat::UINT: return "usampler2D";
        case SamplerFormat::SHADOW: return "sampler2DShadow";
      }
    }
    case SamplerType::SAMPLER_3D: {
      switch (format) {
        case SamplerFormat::FLOAT: return "sampler3D";
        case SamplerFormat::INT: return "isampler3D";
        case SamplerFormat::UINT: return "usampler3D";
        case SamplerFormat::SHADOW: return nullptr;
      }
    }
    case SamplerType::SAMPLER_2D_ARRAY: {
      switch (format) {
        case SamplerFormat::FLOAT: return "sampler2DArray";
        case SamplerFormat::INT: return "isampler2DArray";
        case SamplerFormat::UINT: return "usampler2DArray";
        case SamplerFormat::SHADOW: return "sampler2DShadow";
      }
    }
    case SamplerType::SAMPLER_CUBEMAP: {
      switch (format) {
        case SamplerFormat::FLOAT: return "samplerCube";
        case SamplerFormat::INT: return "isamplerCube";
        case SamplerFormat::UINT: return "usamplerCube";
        case SamplerFormat::SHADOW: return "samplerCubeShadow";
      }
    }
  }
}

const char *CodeGenerator::GetMaterialPropertyName(MaterialProperty::Property property) {
  using Property = MaterialProperty::Property;
  switch (property) {
    case Property::BASE_COLOR:           return "BASE_COLOR";
    case Property::ROUGHNESS:            return "ROUGHNESS";
    case Property::REFLECTANCE:          return "REFLECTANCE";
    case Property::METALLIC:             return "METALLIC";
    case Property::EMISSIVE:             return "EMISSIVE";
    case Property::NORMAL:               return "NORMAL";
  }
}

const char *CodeGenerator::GetRenderPathDefine(RenderPath render_path) {
  switch (render_path) {
    case RenderPath::DEFERRED:          return "RENDER_PATH_DEFERRED";
    case RenderPath::FORWARD:           return "RENDER_PATH_FORWARD";
  }
}
}