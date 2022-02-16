//
// Created by Glodxy on 2022/2/16.
//
#include <string>
#include <iostream>
#include <fstream>
#include "Utils/Image/Cubemap/CubemapIBL.h"
#include "Utils/Image/Cubemap/CubemapUtils.h"
#include "Utils/Image/LinearImage.h"
#include "Utils/Image/ColorTransform.h"
#include "Utils/Image/ImageEncoder.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "Utils/Path/PathUtils.h"
using namespace our_graph;
int main(int argc, char** argv) {
//  if (argc != 1) {
//    return -1;
//  }
  static std::string prefix[6] = {"right_", "left_", "top_", "bottom_", "front_", "back_"};

  std::string input_file("default.jpg");
  int w = 0, h = 0;
  std::string tmp_file = "0" + prefix[0] + input_file;
  if (!utils::PathUtils::CheckFileExist(tmp_file)) {
    std::cerr << tmp_file <<" not exist!";
    return -1;
  }
  stbi_info(tmp_file.c_str(), &w, &h, nullptr);

  if (w != h) {
    std::cerr<< "not cube map! w != h";
    return -1;
  }
  image::Cubemap base = image::CubemapUtils::CreateCubemap(w);


  // 初始化cubemap
  for (int i = 0; i < 6; ++i) {
    std::string file_name = "0" + prefix[i] + input_file;
    int n;
    unsigned char* file_data = stbi_load(input_file.c_str(), &w, &h, &n, 3);
    image::LinearImage linear_image = image::ToLinear(w, h, 3 * w, file_data);
    image::CubemapUtils::SetFaceFromSingleImage(base, (image::Cubemap::Face)i, linear_image);
  }

  std::vector<image::Cubemap> levels;
  levels.push_back(std::move(base));
  levels[0].MakeSeamless();


  image::CubemapUtils::GenerateMipmaps(levels);

  // 直接返回256
  image::Cubemap diffuse_irradiance = image::CubemapUtils::CreateCubemap(256);
  image::CubemapIBL::DiffuseIrradiance(diffuse_irradiance, levels);


  // 输出
  for (int i = 0; i < 6; ++i) {
    std::ofstream os("0"+prefix[i]+"output.png", std::ios::binary | std::ios::trunc);
    image::ImageEncoder::Encode(os, image::ImageEncoder::Format::PNG, diffuse_irradiance.GetImageForFace((image::Cubemap::Face)i));
  }

  return 0;
}