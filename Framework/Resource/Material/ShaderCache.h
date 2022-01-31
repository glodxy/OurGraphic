//
// Created by Glodxy on 2021/12/18.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERCACHE_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERCACHE_H_
#include <string>
#include <map>
#include <vector>
#include "include/GlobalEnum.h"
#include "shaderc/shaderc.hpp"
namespace our_graph {
class ShaderCache {
 public:
  /**
   * 初始化shader数据
   * */
  static void Init();

  /**
   * 获取模块的内容
   * */
  static std::string GetModuleContent(uint32_t module_key);

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
   * @param module: 使用的模块,会将内容加在目标前方
   * */
  static std::string GetDataFromFile(const std::string& file_path, uint32_t module = 0);

  /**
   *
   * */

  static std::vector<uint32_t> CompileFile(const std::string& source_name,
                                           shaderc_shader_kind kind,
                                           const std::string& source,
                                           bool optimize = false);

  /**
   * 获取编译好的数据
   * @note:仅用于global shader
   * */
  static std::vector<uint32_t> GetCompiledData(const std::string& file_name,
                                               uint32_t module_key = 0);

 protected:
  static shaderc_shader_kind GetShaderKind(const std::string& file_path);
  static std::string LoadFromFile(const std::string& file_path);
  // shader的variant key对应的text
  // 使用时进行链接
  static std::map<uint8_t, std::string> shader_variant_data_;
  // shader文件对应的数据
  static std::map<std::string, std::string> shader_file_data_;
  // shader编译后的数据
  // <<file, module_key>, data>
  static std::map<std::pair<std::string, uint32_t>, std::vector<uint32_t>> shader_compiled_data_;
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERCACHE_H_
