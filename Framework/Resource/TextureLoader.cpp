//
// Created by Glodxy on 2022/2/9.
//

#include "Resource/include/TextureLoader.h"
#include "Utils/Path/PathUtils.h"
#include "Utils/OGLogging.h"
#include "Resource/include/Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
namespace our_graph {
std::map<std::string, unsigned char*> TextureLoader::data_cache_ = {};
static std::string GetFileNameFromPath(const std::string& path) {
  auto pos = path.find_last_of('/');
  return path.substr(pos + 1);
}

static std::string GetFileParentPath(const std::string& path) {
  auto pos = path.find_last_of('/');
  return path.substr(0, pos + 1);
}

Texture *TextureLoader::LoadCubeMap(Driver *driver, const std::string &path) {
  Texture* res;
  if (LoadCubemapLevel(driver, &res, path, 0, false)) {
    return res;
  }
  return nullptr;
}

static uint32_t GetTargetChannel(const std::string& path) {
  if (path.find(".jpg") != std::string::npos) {
    return 3;
  }
  if (path.find(".png") != std::string::npos) {
    return 4;
  }
  return 3;
}

static Texture::InternalFormat GetTargetFormat(uint32_t channel) {
  switch (channel) {
    case 3: return Texture::InternalFormat ::RGB8;
    case 4: return Texture::InternalFormat ::RGBA8;
  }
  LOG_ERROR("TextureLoader", "Cannot GetFormat for channel[{}]", channel);
  return Texture::InternalFormat ::RGB8;
}

static Texture::Format GetTargetTexFormat(int channel) {
  switch (channel) {
    case 3: return Texture::Format ::RGB;
    case 4: return Texture::Format::RGBA;
  }
  LOG_ERROR("TextureLoader", "Cannot GetTexFormat for channel[{}]", channel);
  return Texture::Format::RGB;
}

bool TextureLoader::LoadCubemapLevel(Driver* driver, Texture **tex, const std::string &path, size_t level, bool has_mipmap) {
  static std::string prefix[6] = {"right_", "left_", "top_", "bottom_", "front_", "back_"};
  size_t size = 0;
  size_t num_levels = 1;
  uint32_t channels = GetTargetChannel(path);
  {
    int w, h;
    // 随便取一个面的文件
    std::string face_path = GetFileParentPath(path) + std::to_string(level) + prefix[0] + GetFileNameFromPath(path);
    if (!utils::PathUtils::CheckFileExist(face_path)) {
      LOG_ERROR("TextureLoader", "LoadCubemap[{}] failed! file[{}] not exist!",
                path, face_path);
      return false;
    }
    stbi_info(face_path.c_str(), &w, &h, nullptr);
    // 要求每个面必须是正方形纹理
    if (w != h) {
      LOG_ERROR("TextureLoader", "LoadCubemap[{}] failed! file[{}] w != h",
                path, face_path);
      return false;
    }
    size = w;
    // 使用mipmap时，需要计算level等级来创建tex
    if (has_mipmap) {
      num_levels = (size_t) std::log2(size) + 1;
    }

    if (level == 0) {
      // level0 表示创建新tex
      *tex = Texture::Builder(driver)
          .Width(size)
          .Height(size)
          .Levels(num_levels)
          .Format(GetTargetFormat(channels))
          .Sampler(Texture::Sampler::SAMPLER_CUBEMAP)
          .Usage(TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE)
          .Build();
    }
  }
  // 计算每个面的字节大小，每个像素32位
  const size_t face_size = size * size * sizeof(uint8_t) * channels;

  Texture::FaceOffsets offsets;
  Texture::PixelBufferDescriptor buffer(
      malloc(face_size * 6), face_size * 6,
      GetTargetTexFormat(channels), Texture::Type::FLOAT,
      [](void* data, uint32_t size, void* user) {
        free(data);
      });

  bool success = true;
  uint8_t* p = static_cast<uint8_t*>(buffer.buffer_);

  // 处理cubemap的6个面
  for (int i = 0; i < 6; ++i) {
    offsets[i] = face_size * i;
    std::string face_name = GetFileParentPath(path) + std::to_string(level) + prefix[i] + GetFileNameFromPath(path);
    if (!utils::PathUtils::CheckFileExist(face_name)) {
      LOG_ERROR("TextureLoader", "Cubemap file[{}] not exist!",
                face_name);
      success = false;
      break;
    }

    int w, h, n;
    unsigned char* data = stbi_load(face_name.c_str(), &w, &h, &n, channels);
    if (w != h || w != size) {
      LOG_ERROR("TextureLoader", "Cubemap file[{}] size[{}] error! expect [{}]",
                face_name, w, size);
      success = false;
      break;
    }

    if (!data || n != channels) {
      LOG_ERROR("TextureLoader", "Cubemap file[{}] decode failed! channel:{}",
                face_name, n);
      success = false;
      break;
    }

    memcpy(p + offsets[i], data, w * h * sizeof(uint8_t) * channels);
    stbi_image_free(data);
  }
  // todo:暂时只处理level0

  if (success) {
    // 成功了则设置对应的buffer
    (*tex)->SetImage(level, std::move(buffer), offsets);
    return true;
  }

  return false;
}

void TextureLoader::LoadFromFile(const std::string &file_path) {
  int w, h, n;
  unsigned char* data = stbi_load(file_path.c_str(), &w, &h, &n, 0);
  data_cache_[file_path] = data;
}

}