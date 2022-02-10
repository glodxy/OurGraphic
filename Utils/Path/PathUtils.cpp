//
// Created by Glodxy on 2022/2/10.
//

#include "PathUtils.h"
#include <sys/stat.h>
namespace our_graph::utils {

bool PathUtils::CheckFileExist(const std::string &file_path) {
  struct stat file;
  return stat(file_path.c_str(), &file) == 0;
}

}