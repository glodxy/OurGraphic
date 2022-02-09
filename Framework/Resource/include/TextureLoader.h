//
// Created by Glodxy on 2022/2/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TEXTURELOADER_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TEXTURELOADER_H_
#include <string>
#include <map>
#include <vector>
namespace our_graph {

class TextureLoader {
 public:

 private:
  /**
   * @param file_path:绝对路径
   * */
  static void LoadFromFile(const std::string& file_path);

  static std::map<std::string, const unsigned char*> data_cache_;
};

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TEXTURELOADER_H_
