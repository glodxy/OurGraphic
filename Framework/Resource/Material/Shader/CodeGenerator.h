//
// Created by Glodxy on 2021/12/20.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_CODEGENERATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_CODEGENERATOR_H_
#include <sstream>
#include "Backend/include/DriverEnum.h"
#include "Backend/include/Program.h"
#include "Framework/Resource/include_internal/MaterialEnum.h"
#include "Framework/Resource/Material/UniformBlock.h"
#include "Framework/Resource/Material/SamplerBlock.h"
#include "Framework/include/GlobalEnum.h"
namespace our_graph {
class CodeGenerator {
  using ShaderType = Program::ShaderType;
 public:
  CodeGenerator(std::stringstream& ss, ShaderType shader_type);

  // 生成换行符
  void GenerateSeparator();

  /**
   * 生成变量
   * 对于 vs，为layout(location = idx) out xxx;
   * 对于 fs, 为layout(location = idx) in xxx;
   * */
  void GenerateVariable(const std::string& name,
                        const std::string& type,
                        size_t idx);

  /**
   * 生成宏定义
   * */
  void GenerateDefine(const std::string& name, bool value);
  void GenerateDefine(const std::string& name, uint32_t value);
  void GenerateDefine(const std::string& name, const std::string& value);
  void GenerateIndexedDefine(const std::string& name, uint32_t index, uint32_t value);
 public:
  // 生成如version之类的head信息
  void GenerateHead();
  // 生成shader的vs/fs的input
  void GenerateShaderInput(const AttributeBitset& attributes);
  // 生成postprocess的input
  void GeneratePostProcessInput();

  // 只有post process会使用
  void GenerateOutput();

  // 生成uniform
  void GenerateUniforms(uint32_t binding, const UniformBlock& uniform_block);
  // 生成sampler
  void GenerateSamplers(uint32_t binding, const SamplerBlock& sampler_block);
 private:
  // 获取uniform 字段类型对应的字符串
  static const char* GetUniformTypeName(UniformBlock::Type type);
  // 获取sampler的类型名字
  static const char* GetSamplerTypeName(SamplerType type, SamplerFormat format, bool multi_sample);
  std::stringstream ss_;
  // 标志是vs还是fs
  ShaderType shader_type_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_CODEGENERATOR_H_
