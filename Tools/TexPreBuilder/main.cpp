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

  std::string input_file("debug.png");
  image::LinearImage tmp;
  image::Cubemap base = image::CubemapUtils::CreateCubemap(tmp, 256);


  int w, h, n;
  unsigned char* file_data = stbi_load(input_file.c_str(), &w, &h, &n, 3);
  image::LinearImage linear_image = image::FromSRGBToLinear(w, h, 3 * w, file_data);
  stbi_image_free(file_data);

  std::cout<<"start set face for base level cubemap"<<std::endl;
  image::CubemapUtils::CrossToCubemap(base, linear_image);
  // 初始化cubemap
//  for (int i = 1; i < 2; ++i) {
//    std::string file_name = "0" + prefix[i] + input_file;
//    int n;
//    unsigned char* file_data = stbi_load(file_name.c_str(), &w, &h, &n, 3);
//    image::LinearImage linear_image = image::FromRGBToLinear(w, h, 3 * w, file_data);
//    stbi_image_free(file_data);
//    image::CubemapUtils::SetFaceFromSingleImage(base, (image::Cubemap::Face)i, linear_image);
//  }

  std::cout<<"start make base level seamless"<<std::endl;
  std::vector<image::Cubemap> levels;
  levels.push_back(std::move(base));
  levels[0].MakeSeamless();


  std::cout<<"start generate mipmaps!"<<std::endl;
  //image::CubemapUtils::GenerateMipmaps(levels);

  // 直接返回256
  image::LinearImage whole_image;
  image::Cubemap diffuse_irradiance = image::CubemapUtils::CreateCubemap(whole_image, 256);

  std::cout<<"start generate diffuse irradiance!"<<std::endl;
  //image::CubemapIBL::DiffuseIrradiance(diffuse_irradiance, levels);


  std::cout<<"done! start output"<<std::endl;
  // 输出
  for (int i = 0; i < 6; ++i) {
    std::ofstream os("0"+prefix[i]+"output.png", std::ios::binary | std::ios::trunc);
    image::ImageEncoder::Encode(os, image::ImageEncoder::Format::PNG, levels[0].GetImageForFace((image::Cubemap::Face)i));
    os.close();
  }

  //image::LinearImage sub = tmp.Subset(512, 512, 1024, 512);
  std::ofstream os("whole.png", std::ios::binary | std::ios::trunc);
  image::ImageEncoder::Encode(os, image::ImageEncoder::Format::PNG, tmp);
  os.close();
  return 0;
}