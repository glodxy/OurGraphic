//
// Created by Glodxy on 2021/12/18.
//

#include "ShaderCache.h"
#include <fstream>
#include <sstream>

#include "Utils/OGLogging.h"
namespace {
const static const char* kFilePath[] = {
    // todo
};
}


namespace our_graph {

std::string ShaderCache::GetDataFromFile(const std::string &file_path) {
  if (shader_file_data_.find(file_path) != shader_file_data_.end()) {
    return shader_file_data_[file_path];
  }
  std::string text = LoadFromFile(file_path);
  shader_file_data_[file_path] = text;
  return text;
}

std::string ShaderCache::LoadFromFile(const std::string &file_path) {
  std::ifstream file;
  file.open(file_path, std::ios::in);
  if (!file.is_open()) {
    char current_path[1024];
    getcwd(current_path, 1024);
    LOG_ERROR("ShaderCache", "Open {} Failed!", std::string(current_path) + "/" + file_path);
    return "";
  }
  std::istreambuf_iterator<char> begin(file), end;
  std::string text(begin, end);
  file.close();
  return text;
}

std::string ShaderCache::GetModuleContent(uint8_t module_key) {
  uint8_t idx = 0;
  std::stringstream ss;
  for (; idx < ShaderVariantBit::MAX; ++idx) {
    if ((idx & module_key) != 0) {
      ss << shader_variant_data_[idx];
      ss << "\n";
    }
  }
  return ss.str();
}

void ShaderCache::Init() {
   // 加载内置模块的shader
  size_t module_file_size = sizeof(kFilePath) / sizeof(const char*);
  for (size_t idx = 0; idx < module_file_size; ++idx) {
    shader_variant_data_[idx] = LoadFromFile(kFilePath[idx]);
  }
}

}