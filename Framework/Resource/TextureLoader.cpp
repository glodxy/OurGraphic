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

Texture *TextureLoader::LoadCubeMap(Driver *driver, const std::string &path, size_t levels) {
  Texture* res;
  bool has_mipmap = levels > 0;
  if (LoadCubemapLevel(driver, &res, path, 0, has_mipmap, levels)) {
    for (size_t l = 1; l <= levels; ++l) {
      if (!LoadCubemapLevel(driver, &res, path, l, true, levels)) {
        // todo:销毁已有资源
        LOG_ERROR("TextureLoader", "LoadCubemap level[{}] failed!", l);
        return nullptr;
      }
    }
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

bool TextureLoader::LoadCubemapLevel(Driver* driver, Texture **tex, const std::string &path, size_t level, bool has_mipmap, size_t max_level) {
  static std::string prefix[6] = {"right_", "left_", "top_", "bottom_", "front_", "back_"};
  size_t size = 0;
  size_t num_levels = 1;
  uint32_t channels = GetTargetChannel(path);
  {
    int w, h, n;
    // 随便取一个面的文件
    std::string face_path = GetFileParentPath(path) + std::to_string(level) + prefix[0] + GetFileNameFromPath(path);
    if (!utils::PathUtils::CheckFileExist(face_path)) {
      LOG_ERROR("TextureLoader", "LoadCubemap[{}] failed! file[{}] not exist!",
                path, face_path);
      return false;
    }
    // 此处加载获取通道数
    unsigned char* data = stbi_load(face_path.c_str(), &w, &h, &n, channels);
    stbi_image_free(data);
    // 要求每个面必须是正方形纹理
    if (w != h) {
      LOG_ERROR("TextureLoader", "LoadCubemap[{}] failed! file[{}] w != h",
                path, face_path);
      return false;
    }
    if (channels != n) {
      // 此处仅需警告
      LOG_WARN("TextureLoader", "LoadCubemap[{}] desire channel[{}], but get [{}]!",
               path, channels, n);
    }

    channels = n;
    size = w;
    // 使用mipmap时，需要计算level等级来创建tex
    if (has_mipmap) {
      num_levels = max_level > 0 ? max_level + 1 : (size_t) std::log2(size) + 1;
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
      GetTargetTexFormat(channels), Texture::Type::BYTE,
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


Texture * TextureLoader::LoadTexture(Driver *driver, const std::string &path) {
  size_t num_levels = 1;


  int w, h;
  // 随便取一个面的文件
  if (!utils::PathUtils::CheckFileExist(path)) {
    LOG_ERROR("TextureLoader", "LoadTexture[{}] failed! not exist!",
              path);
    return nullptr;
  }
  stbi_info(path.c_str(), &w, &h, nullptr);

  size_t size = std::min(w, h);
  num_levels = (size_t) std::log2(size) + 1;

  // 单纹理直接在加载时获取通道数
  int n;

  unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 0);

  if (!data) {
    LOG_ERROR("TextureLoader", "Cubemap file[{}] decode failed! channel:{}",
              path, n);
    // todo:销毁该tex, 返回null
    return nullptr;
  }

  Texture* tex = Texture::Builder(driver)
      .Width(w)
      .Height(h)
      .Levels(num_levels)
      .Format(GetTargetFormat(n))
      .Sampler(Texture::Sampler::SAMPLER_2D)
      .Usage(TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE)
      .Build();

  if (!tex) {
    LOG_ERROR("TextureLoader", "LoadTexture[{}] failed! Create Texture Failed!", path);
    return nullptr;
  }


  size_t image_size = w * h * sizeof(uint8_t) * n;
  // todo:texture 格式
  Texture::PixelBufferDescriptor buffer(
      malloc(image_size), image_size,
      GetTargetTexFormat(n), Texture::Type::UBYTE,
      [](void* data, uint32_t size, void* user) {
        free(data);
      });
  uint8_t* p = static_cast<uint8_t*>(buffer.buffer_);

  memcpy(p , data, image_size);
  stbi_image_free(data);

  tex->SetImage(0, 0, 0, w, h, std::move(buffer));
  tex->GenerateMipmaps();
  return tex;
}

void TextureLoader::LoadFromFile(const std::string &file_path) {
  int w, h, n;
  unsigned char* data = stbi_load(file_path.c_str(), &w, &h, &n, 0);
  data_cache_[file_path] = data;
}


}