//
// Created by Glodxy on 2021/12/22.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_SHADERGENERATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_SHADERGENERATOR_H_
#include <string>
#include "MaterialInfo.h"
namespace our_graph {
class ShaderGenerator {
 public:
  /**
   * 生成顶点着色器的代码
   * @param material_info：需要使用的材质信息
   * @param module_key：会使用哪些内置模块
   * */
  std::string CreateVertexShader(const MaterialInfo& material_info, uint8_t module_key);

  /**
   * 生成片段着色器的代码
   * @param material_info：需要使用的材质信息
   * @param module_key：会使用哪些内置模块
   * */
  std::string CreateFragShader(const MaterialInfo& material_info, uint8_t module_key);
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_SHADERGENERATOR_H_
