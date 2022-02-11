//
// Created by Glodxy on 2021/12/28.
//

#include "Framework/Resource/include/MaterialCache.h"
#include <fstream>

#define MATERIAL_ROOT_PATH GRAPHIC_ROOT_PATH/material
#define INLINE_STR(R) #R
#define STR(R) INLINE_STR(R)

namespace our_graph {

std::map<std::string, Material*> MaterialCache::cache_;

Material * MaterialCache::GetMaterial(const std::string &file, Driver* driver) {
  if (cache_.find(file) != cache_.end()) {
    return cache_[file];
  }
  // 读取文件内容
  std::ifstream in;
  std::string root_path = STR(MATERIAL_ROOT_PATH);
  std::string path = root_path + "/" + file;
  in.open(path, std::ios::in);
  if (!in.is_open()) {
    LOG_ERROR("ShaderCache", "Open {} Failed!", path);
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