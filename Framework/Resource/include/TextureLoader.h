//
// Created by Glodxy on 2022/2/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TEXTURELOADER_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TEXTURELOADER_H_
#include <string>
#include <map>
#include <vector>
#include "Backend/include/Driver.h"
namespace our_graph {
class Texture;
class TextureLoader {
 public:
  /**
   * 从本地路径加载cube map的Texture
   * @param levels:有多少级文件
   * 应满足规范，
   * @example:
   * path:xxx/A.jpg
   * 则应有文件xxx/0_A.jpg, ..., xxx/5_A.jpg
   * */
  static Texture* LoadCubeMap(Driver* driver, const std::string& path, size_t levels = 0);

  /**
   * 加载2D的texture
   * */
  static Texture* LoadTexture(Driver* driver, const std::string& path);
 private:
  /**
   * @param file_path:绝对路径
   * */
  static void LoadFromFile(const std::string& file_path);

  /**
   * @param has_mipmap:是否存在mipmap
   * */
  static bool LoadCubemapLevel(Driver* driver, Texture** tex, const std::string& path,
                               size_t level, bool has_mipmap = false, size_t max_level = 0);

  static std::map<std::string, unsigned char*> data_cache_;
};

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TEXTURELOADER_H_
