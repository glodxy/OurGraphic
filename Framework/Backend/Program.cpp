//
// Created by Glodxy on 2021/10/10.
//
#include <string>
#include <iostream>
#include "include_internal/Program.h"

namespace our_graph {
Program::Program() noexcept {}
Program::Program(Program &&r) noexcept = default;
Program &Program::operator=(Program &&) noexcept = default;
Program::~Program() noexcept = default;

Program &Program::Diagnostics(const std::string &name, uint8_t variant_key) {
  name_ = name;
  variant_ = variant_key;
  return *this;
}

Program &Program::Shader(ShaderType shader_type, const void *data, size_t size) noexcept {
  ShaderBlob blob(size);
  std::copy_n((const uint8_t*)data, size, blob.data());
  shader_source_[size_t(shader_type)] = std::move(blob);
  return *this;
}

Program &Program::SetUniformBlock(size_t binding_point, std::string uniform_block_name) noexcept {
  uniform_block_info_[binding_point] = std::move(uniform_block_name);
  return *this;
}

Program &Program::SetSamplerGroup(size_t binding_point, const Sampler *samplers, size_t count) noexcept {
  auto& sampler_list = sampler_group_info_[binding_point];
  sampler_list.reserve(count);
  sampler_list.resize(count);
  std::copy_n(samplers, count, sampler_list.data());
  has_samplers_ = true;
  return *this;
}

#if DEBUG
std::ostream & operator<< (std::ostream& out, const Program& p) {
  return out << "Program(" << p.name_.c_str() << ")";
}
#endif
}  // namespace our_graph