//
// Created by Glodxy on 2021/12/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_MATERIALPARSER_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_MATERIALPARSER_H_
#include "Framework/Backend/include/Driver.h"
#include "Framework/Resource/include_internal/MaterialEnum.h"
#include "Framework/Backend/include/Program.h"
#include "Framework/Resource/Material/SamplerBlock.h"
#include "Framework/Resource/Material/UniformBlock.h"
#include "Framework/Resource/Material/Shader/MaterialInfo.h"
#include "json/json.h"
namespace our_graph {
class ShaderBuilder;
/**
 * 该类用于从数据中解析material的相应数据
 * */
class MaterialParser {

 public:
  using ShaderType = Program::ShaderType;
  MaterialParser(Backend backend, const void* data, size_t size);
  MaterialParser(const MaterialParser&) = delete;
  MaterialParser& operator=(const MaterialParser&) = delete;

  bool Parse();

  bool GetVersion(uint32_t& value) const noexcept;
  bool GetName(std::string& value) const noexcept;
  bool GetUniformBlock(UniformBlock& value) const noexcept;
  bool GetSamplerBlock(SamplerBlock& value) const noexcept;
  bool GetSamplerBindingMap(SamplerBindingMap& map) const noexcept;

  bool GetDepthWrite(bool& value) const noexcept;
  bool GetDoubleSided(bool& value) const noexcept;
  bool GetColorWrite(bool& value) const noexcept;
  bool GetDepthTest(bool& value) const noexcept;
  bool GetCullingMode(CullingMode& culling_mode) const noexcept;
  //bool GetTransparencyMode(TransparencyMode& value) const noexcept;
  bool GetMaterialDomain(MaterialDomain& value) const noexcept;

  bool GetShadingModel(ShadingModel& value) const noexcept;
  bool GetBlendingModel(BlendingMode& value) const noexcept;
  bool GetMaskThreshold(float& value) const noexcept;
  bool GetRequiredAttributes(AttributeBitset& value) const noexcept;
  bool GetRefractionMode(RefractionMode& value) const noexcept;
  bool GetRefractionType(RefractionType& value) const noexcept;
  bool GetCustomDepthShaderSet(bool& value) const noexcept;

  uint32_t GetModuleKey() const noexcept;


  bool GetShader(ShaderBuilder& builder,
                 ShaderType type);


 private:
  bool ParseSamplers();
  bool ParseUniforms();

  void ParseVersion() noexcept;
  void ParseName()noexcept;

  void ParseDepthWrite() noexcept;
  void ParseDoubleSided() noexcept;
  void ParseColorWrite() noexcept;
  void ParseDepthTest() noexcept;
  void ParseCullingMode() noexcept;
  //void ParseTransparencyMode() noexcept;
  void ParseMaterialDomain() noexcept;

  void ParseShadingModel() noexcept;
  void ParseBlendingModel() noexcept;
  void ParseMaskThreshold() noexcept;
  void ParseRequiredAttributes() noexcept;
  void ParseRefractionMode() noexcept;
  void ParseRefractionType() noexcept;
  void ParseCustomDepthShaderSet() noexcept;

  /**
   * 解析shader
   * 1. 先通过InterParseModuleKey得到会使用的模块
   * 2. 通过InterParseShader调用ShaderGenerator得到目标的shader内容
   * */
  void ParseShader() noexcept;
  // 解析对应的类型shader的text
  std::string InterParseShader(ShaderType type, uint32_t module_key) noexcept;
  // 解析得到使用的模块key
  uint32_t InterParseModuleKey() noexcept;
  // 解析shader间传递的变量
  void ParseVariables() noexcept;
  std::string name_;
  uint32_t version_;

  Json::Reader reader_;
  Json::Value root_;
  Json::Value params_;
  Json::Value samplers_;
  Json::Value uniforms_;
  Json::Value shaders_;
  UniformBlock uniform_block_;
  SamplerBlock sampler_block_;

  // 解析得到的材质信息
  MaterialInfo material_info_;

  uint32_t module_key_;
  // 着色器的内容
  std::string vertex_shader_text_;
  std::string frag_shader_text_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_MATERIALPARSER_H_
