//
// Created by Glodxy on 2021/12/28.
//

#include "MaterialCache.h"
#include <fstream>
namespace our_graph {
std::map<std::string, Material*> MaterialCache::cache_;

Material * MaterialCache::GetMaterial(const std::string &file, Driver* driver) {
  if (cache_.find(file) != cache_.end()) {
    return cache_[file];
  }
  // 读取文件内容
  std::ifstream in;
  in.open(file, std::ios::in);
  if (!in.is_open()) {
    char current_path[1024];
    getcwd(current_path, 1024);
    LOG_ERROR("ShaderCache", "Open {} Failed!", std::string(current_path) + "/" + file);
    return nullptr;
  }
  std::istreambuf_iterator<char> begin(in), end;
  std::string text(begin, end);
  in.close();
  Material* mat = Material::Builder(driver).Data(text.data(), text.size()).Build();
  cache_[file] = mat;
  return mat;
}

}