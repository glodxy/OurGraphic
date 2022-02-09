//
// Created by Glodxy on 2021/12/22.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_SHADERGENERATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_SHADERGENERATOR_H_
#include <string>
#include "Framework/Backend/include/Program.h"
#include "MaterialInfo.h"
namespace our_graph {
class ShaderGenerator {
  using ShaderType = Program::ShaderType;
 public:
  // 总函数，分发至type对应的生成函数
  /**
   * @param subpass_idx:该shader所属的pass
   * */
  std::string CreateShaderText(ShaderType type, const MaterialInfo& material_info,
                               uint32_t module_key, uint8_t subpass_idx);

  std::string CreateGlobalShaderText(ShaderType type, uint32_t module_key, GlobalShaderType shader_type,
                                     UniformBlock uniform, SamplerBlock sampler, SamplerBindingMap binding_map);
 private:
  /**
   * 生成顶点着色器的代码
   * @param material_info：需要使用的材质信息
   * @param module_key：会使用哪些内置模块
   * */
  std::string CreateVertexShader(const MaterialInfo& material_info,
                                 uint8_t module_key, uint8_t subpasss_idx);

  /**
   * 生成片段着色器的代码
   * @param material_info：需要使用的材质信息
   * @param module_key：会使用哪些内置模块
   * */
  std::string CreateFragShader(const MaterialInfo& material_info,
                               uint8_t module_key, uint8_t subpass_idx);

  std::string CreateGlobalVertexShader(uint32_t module_key, GlobalShaderType shader_type,
                                       UniformBlock uniform, SamplerBlock sampler, SamplerBindingMap binding_map);

  std::string CreateGlobalFragShader(uint32_t module_key, GlobalShaderType shader_type,
                                     UniformBlock uniform, SamplerBlock sampler, SamplerBindingMap binding_map);

  std::string GetShadingModelDefine(ShadingModel model);
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_SHADERGENERATOR_H_
