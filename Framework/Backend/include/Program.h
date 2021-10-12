//
// Created by Glodxy on 2021/10/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_PROGRAM_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_PROGRAM_H_
#include <array>
#include <vector>
#include <string>
#include "Framework/Backend/include_internal/DriverEnum.h"

namespace our_graph {
class Program {
 public:
  static constexpr size_t SHADER_TYPE_COUNT = 2;
  static constexpr size_t BINDING_COUNT = CONFIG_BINDING_COUNT;

  enum class ShaderType : uint8_t {
    VERTEX = 0,
    FRAGMENT = 1
  };

  struct Sampler {
    std::string name = ""; // shader中使用的采样器名称
    uint16_t binding = 0; // shader中绑定的槽位置
    bool strict = false; // 是否必须绑定纹理
  };

  using SamplerGroupInfo = std::array<std::vector<Sampler>, BINDING_COUNT>;
  using UniformBlockInfo = std::array<std::string, BINDING_COUNT>;
  using ShaderBlob = std::vector<uint8_t>;

  Program() noexcept;
  Program(const Program& r) = delete;
  Program& operator=(const Program&) = delete;
  Program(Program&& r) noexcept;
  Program& operator=(Program&& r) noexcept;
  ~Program()noexcept;

  /**
   *设置material的名称与变量
   * */
  Program& Diagnostics(const std::string& name, uint8_t variant_key);

  /**
   * 设置对应的shader
   * */
  Program& Shader(ShaderType shader_type, const void* data, size_t size) noexcept;

  /**
   * 将指定的uniform block绑定到指定的槽上
   * */
  Program& SetUniformBlock(size_t binding_point, std::string uniform_block_name) noexcept;

  /**
   * 将指定的系列sampler绑定至指定的槽
   * */
  Program& SetSamplerGroup(size_t binding_point, const Sampler* samplers, size_t count) noexcept;

  Program& WithVertexShader(const void* data, size_t size) {
    return Shader(ShaderType::VERTEX, data, size);
  }

  Program& WithFragmentShader(const void* data, size_t size) {
    return Shader(ShaderType::FRAGMENT, data, size);
  }

  std::array<ShaderBlob, SHADER_TYPE_COUNT> const& GetShadersSource() const noexcept {
    return shader_source_;
  }

  UniformBlockInfo const& GetUniformBlockInfo() const noexcept { return uniform_block_info_; }

  SamplerGroupInfo const& GetSamplerGroupInfo() const { return sampler_group_info_; }

  const std::string& GetName() const noexcept { return name_; }

  uint8_t GetVariant() const noexcept { return variant_; }

  bool HasSamplers() const noexcept { return has_samplers_; }

 private:
  UniformBlockInfo uniform_block_info_ = {};
  SamplerGroupInfo sampler_group_info_ = {};
  std::array<ShaderBlob , SHADER_TYPE_COUNT> shader_source_;
  std::string name_;
  bool has_samplers_ = false;
  /***/
  uint8_t variant_;

#if DEBUG
  friend std::ostream& operator<< (std::ostream& out, const Program& p);
#endif
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_PROGRAM_H_
