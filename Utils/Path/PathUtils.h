//
// Created by Glodxy on 2022/2/10.
//

#ifndef OUR_GRAPHIC_UTILS_PATH_PATHUTILS_H_
#define OUR_GRAPHIC_UTILS_PATH_PATHUTILS_H_
#include <string>
namespace our_graph::utils {
class PathUtils {
 public:
  static bool CheckFileExist(const std::string& file_path);
};
}
#endif //OUR_GRAPHIC_UTILS_PATH_PATHUTILS_H_
