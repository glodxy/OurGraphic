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
  bool GetRenderPath(RenderPath& value) const noexcept;
  bool GetDefaultParamValue(ParamValueList& list) const noexcept;

  uint32_t GetModuleKey() const noexcept;


  bool GetShader(ShaderBuilder& builder,
                 ShaderType type,
                 uint8_t subpass_idx);

 private:
  bool ParseParams() noexcept;
  // 判断该字段是否是内置property，是的话返回true以及对应的property索引
  bool IsProperty(const std::string& name,
                  size_t size,
                  UniformType type,
                  MaterialProperty::Property& prop);

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
  void ParseRenderPath() noexcept;

  /**
   * 解析subpass的相关信息
   * 1. 调用InterParseModuleKey解析module key
   * 2. 通过ParseShader解析所有subpass中的shader info
   * */
  void ParseSubpass() noexcept;
  /**
   * 解析shader
   * 解析subpass中的shader info并写入material info
   * */
  void ParseShader(SubpassInfo& pass_info, Json::Value node) noexcept;
  // 解析得到使用的模块key
  uint32_t InterParseModuleKey() noexcept;

  /**
   * 通过material info生成所有subpas的shader text
   * 调用InterGenerateShader来生成
   * */
  void GenerateShaderText();
  // 解析对应的类型shader的text
  std::string InterGenerateShader(ShaderType type,
                               uint32_t module_key,
                               uint8_t subpass_idx) noexcept;

  // 解析shader间传递的变量
  void ParseVariables() noexcept;
  std::string name_;
  uint32_t version_;

  Json::Reader reader_;
  Json::Value root_;

  // 解析得到的材质信息
  MaterialInfo material_info_;



  uint32_t module_key_;
  // 着色器的内容
  std::vector<std::pair<std::string, std::string>> shader_text_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_MATERIALPARSER_H_
