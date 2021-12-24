//
// Created by Glodxy on 2021/12/22.
//
#include "ShaderGenerator.h"
#include <sstream>

#include "CodeGenerator.h"
#include "Framework/Backend/include/Program.h"
namespace our_graph {
std::string ShaderGenerator::CreateVertexShader(const MaterialInfo &material_info, uint8_t module_key) {
  using ShaderType = Program::ShaderType;
  std::stringstream ss;
  CodeGenerator cg(ss, ShaderType::VERTEX);

  cg.GenerateHead();
}

}  // namespace our_graph