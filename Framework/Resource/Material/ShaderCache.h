//
// Created by Glodxy on 2021/12/18.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERCACHE_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERCACHE_H_
#include <string>
#include <map>
#include "include/GlobalEnum.h"

namespace our_graph {
class UniformBlock;
class SamplerBlock;
class ShaderCache {
 public:
  /**
   * 初始化shader数据
   * */
  static void Init();

  /**
   * 获取模块的输入
   * @param module_key:使用的模块
   * */
  static std::string GetModuleInput(uint8_t module_key);

  /**
   * 获取模块的内容
   * */
  static std::string GetModuleContent(uint8_t module_key);
  /**
   * 获取 variant key对应的shader数据
   * */
  static std::string GetData(uint8_t key);

  /**
   * 从文件加载内容
   * @param file_path:文件的路径
   * */
  static std::string GetDataFromFile(const std::string& file_path);
  static std::string GetShaderText(const std::string& file_name,
                                   const UniformBlock& uniform,
                                   const SamplerBlock& sampler);
 protected:



  // shader的variant key对应的text
  // 使用时进行链接
  static std::map<uint8_t, std::string> shader_variant_data_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERCACHE_H_
