//
// Created by Glodxy on 2021/12/20.
//

#include "CodeGenerator.h"

namespace our_graph {
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

void CodeGenerator::GenerateVariable(const std::string &name, const std::string &type, size_t idx) {
  if (!name.empty() && !type.empty()) {
    if (shader_type_ == ShaderType::VERTEX) {
      ss_ << "\n#define VARIABLE_CUSTOM" << idx << " " << name << "\n";
      ss_ << "\n#define VARIABLE_CUSTOM_AT" << idx << " variable_" << name << "\n";
      ss_ << "layout(location=" << idx << ") out " << type << " variable_" << name << "\n";
    } else {
      ss_ << "layout(location=" << idx << ") in " << type << " variable_" << name << "\n";
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
}
}