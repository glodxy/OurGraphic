//
// Created by Glodxy on 2022/2/9.
//

#include "Resource/include/TextureLoader.h"
#include "stb/stb_image.h"
namespace our_graph {
std::map<std::string, const unsigned char*> TextureLoader::data_cache_ = {};

void TextureLoader::LoadFromFile(const std::string &file_path) {
  int w, h, n;
  const unsigned char* data = stbi_load(file_path.c_str(), &w, &h, &n, 0);
  data_cache_[file_path] = data;
}

}