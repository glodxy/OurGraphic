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
#include "json/json.h"
namespace our_graph {
class ShaderBuilder;
/**
 * 该类用于从数据中解析material的相应数据
 * */
class MaterialParser {
  using ShaderType = Program::ShaderType;
 public:
  MaterialParser(Backend backend, const void* data, size_t size);
  MaterialParser(const MaterialParser&) = delete;
  MaterialParser& operator=(const MaterialParser&) = delete;

  bool Parse();

  bool GetVersion(uint32_t& value) const noexcept;
  bool GetName(std::string& value) const noexcept;
  bool GetUniformBlock(UniformBlock& value) const noexcept;
  bool GetSamplerBlock(SamplerBlock& value) const noexcept;

  bool GetDepthWrite(bool& value) const noexcept;
  bool GetDoubleSided(bool& value) const noexcept;
  bool GetColorWrite(bool& value) const noexcept;
  bool GetDepthTest(bool& value) const noexcept;
  bool GetCullingMode(CullingMode& culling_mode) const noexcept;
  bool GetTransparencyMode(TransparencyMode& value) const noexcept;
  bool GetMaterialDomain(MaterialDomain& value) const noexcept;

  bool GetShadingModel(ShadingModel& value) const noexcept;
  bool GetBlendingModel(BlendingMode& value) const noexcept;
  bool GetMaskThreshold(float& value) const noexcept;
  bool GetRequiredAttributes(AttributeBitset& value) const noexcept;
  bool GetRefractionMode(RefractionMode& value) const noexcept;
  bool GetRefractionType(RefractionType& value) const noexcept;
  bool GetCustomDepthShaderSet(bool& value) const noexcept;

  bool GetShader(ShaderBuilder& builder, uint8_t variant_key,
                 ShaderType type);

 private:
  bool ParseSamplers(SamplerBlock& sampler_block);
  bool ParseUniforms(UniformBlock& uniform_block);

  void InitUniformBlock();
  void InitSamplerBlock();
  std::string name_;
  Json::Reader reader_;
  Json::Value root_;
  Json::Value params_;
  Json::Value samplers_;
  Json::Value uniforms_;
  Json::Value shaders_;
  UniformBlock uniform_block_;
  SamplerBlock sampler_block_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_MATERIALPARSER_H_