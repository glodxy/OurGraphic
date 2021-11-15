//
// Created by Glodxy on 2021/10/12.
//

#include "ShaderCache.h"
#include "shaderc/shaderc.hpp"
#include <fstream>
#include <string>
#include "Utils/OGLogging.h"


namespace our_graph {
ShaderCache::ShaderCache() {

}

ShaderCache::~ShaderCache() {
}

ShaderCache::ShaderBuilder::ShaderBuilder(std::string shader_name, ShaderCache *cache) {
  shader_name_ = shader_name;
  shader_cache_ = cache;
}

ShaderCache::ShaderBuilder &ShaderCache::ShaderBuilder::Vertex(std::string file_path) {
  vertex_file_path_ = file_path;
  return *this;
}

ShaderCache::ShaderBuilder &ShaderCache::ShaderBuilder::Frag(std::string file_path) {
  frag_file_path_ = file_path;
  return *this;
}

std::vector<uint32_t> ShaderCache::ShaderBuilder::CompileFile(Program::ShaderType type, std::string file_path) {
  shaderc_shader_kind kind =
      type == Program::ShaderType::VERTEX ? shaderc_glsl_vertex_shader : shaderc_glsl_fragment_shader;
  std::ifstream file;
  file.open(file_path, std::ios::in);
  if (!file.is_open()) {
    char current_path[1024];
    getcwd(current_path, 1024);
    LOG_ERROR("ShaderCache", "Open {} Failed!", std::string(current_path) + "/" + file_path);
    return std::vector<uint32_t >();
  }
  std::istreambuf_iterator<char> begin(file), end;
  std::string text(begin, end);
  file.close();

  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  auto module =
      compiler.CompileGlslToSpv(text, kind, file_path.c_str(), options);
  LOG_INFO("ShaderCache", "Compile:{}", text);
  if (module.GetCompilationStatus() !=
      shaderc_compilation_status_success) {
    LOG_ERROR("ShaderCache", "Compile {} Failed! code:{}, msg:{}",
              file_path, module.GetCompilationStatus(),
              module.GetErrorMessage());
    return std::vector<uint32_t >();
  }

  return {module.cbegin(), module.cend()};
}

Program ShaderCache::ShaderBuilder::Build() {
  Program program;
  program.Diagnostics(shader_name_, 0);
  auto vertex_buf = CompileFile(Program::ShaderType::VERTEX, vertex_file_path_);
  LOG_INFO("ShaderCache", "Compiled Vertex Size:{}", vertex_buf.size());
  program.Shader(Program::ShaderType::VERTEX, vertex_buf.data(), sizeof(uint32_t) * vertex_buf.size());

  // 读取frag
  auto frag_buf = CompileFile(Program::ShaderType::FRAGMENT, frag_file_path_);

  program.WithFragmentShader(frag_buf.data(), sizeof(uint32_t) * frag_buf.size());

  return program;
}

}  // namespace our_graph