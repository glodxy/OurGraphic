//
// Created by Glodxy on 2021/12/18.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERCACHE_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERCACHE_H_
#include <string>
#include <map>
#include <vector>
#include "include/GlobalEnum.h"

namespace our_graph {
class ShaderCache {
 public:
  /**
   * 初始化shader数据
   * */
  static void Init();

  /**
   * 获取模块的uniform
   * @param module_key:使用的模块
   * */
  static std::string GetModuleUniform(uint8_t module_key);

  /**
   * 获取模块的内容
   * */
  static std::string GetModuleContent(uint8_t module_key);

  /**
   * 获得内置的getter数据（用于访问参数）
   * */
  static std::string GetGetterData();

  /**
   * 获得vs默认的输入shader
   * */
  static std::string GetVsInputData();

  /**
   * 获得fs默认的输入shader
   * */
  static std::string GetFsInputData();
  /**
   * 从文件加载内容
   * @param file_path:文件的路径
   * */
  static std::string GetDataFromFile(const std::string& file_path);

 protected:
  static std::string LoadFromFile(const std::string& file_path);
  // shader的variant key对应的text
  // 使用时进行链接
  static std::map<uint8_t, std::string> shader_variant_data_;
  // material的shader文件
  static std::map<std::string, std::string> shader_file_data_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERCACHE_H_
