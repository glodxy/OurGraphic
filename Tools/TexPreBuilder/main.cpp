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

static std::string prefix[6] = {"right_", "left_", "top_", "bottom_", "front_", "back_"};

void OutputLUT() {
  image::LinearImage lut_image(512, 512, 3);
  image::CubemapIBL::LUT(lut_image);
  //image::LinearImage sub = tmp.Subset(512, 512, 1024, 512);
  std::ofstream os("lut.png", std::ios::binary | std::ios::trunc);
  image::ImageEncoder::Encode(os, image::ImageEncoder::Format::PNG, lut_image);
  os.close();
}

/**
 * 输出prefilter的贴图
 * @param dst_base_dim:目标level0 的dim
 * @param levels：输出多少级的roughness
 * */
void OutputPrefilter(size_t levels, size_t dst_base_dim, const std::vector<image::Cubemap>& srcs) {
  size_t dim = dst_base_dim;
  size_t samples = 1024;
  for (size_t l = 0; l <= levels; ++l) {
    if (dim == 0) {
      std::cerr<< "prefilter[" << l <<"] error! dim 0!";
      break;
    }
    image::LinearImage whole_image;
    image::Cubemap dst = image::CubemapUtils::CreateCubemap(whole_image, dim);

    std::cout<<"start generate prefilter:"<< dim << std::endl;
    const float roughness = (float)l / (float)levels;
    image::CubemapIBL::RoughnessFilter(dst, srcs, roughness, samples, {1, 1, 1}, true);

    dst.MakeSeamless();
    std::cout<<"done! start output"<<std::endl;
    // 输出
    for (int i = 0; i < 6; ++i) {
      std::ofstream os("output/"+std::to_string(l)+prefix[i]+"prefilter.png", std::ios::binary | std::ios::trunc);
      image::ImageEncoder::Encode(os, image::ImageEncoder::Format::PNG, dst.GetImageForFace((image::Cubemap::Face)i));
      os.flush();
      os.close();
    }
    std::string whole_name = std::to_string(l) + "whole_prefilter.png";
    std::ofstream os(whole_name, std::ios::binary | std::ios::trunc);
    image::ImageEncoder::Encode(os, image::ImageEncoder::Format::PNG, whole_image);
    os.flush();
    os.close();

    std::cout<< "prefilter[" << l <<"] finished!"<<std::endl;

    dim >>= 1;
    if (levels >= 2) {
      samples *= 2;
    }
  }
}

void OutputDiffuse(size_t dst_dim, const std::vector<image::Cubemap>& srcs) {
  image::LinearImage whole_image;
  image::Cubemap dst = image::CubemapUtils::CreateCubemap(whole_image, dst_dim);

  std::cout << "start generate diffuse irradiance!" << std::endl;
  image::CubemapIBL::DiffuseIrradiance(dst, srcs);
  dst.MakeSeamless();

  std::cout<<"done! start output"<<std::endl;
  // 输出
  for (int i = 0; i < 6; ++i) {
    std::ofstream os("0"+prefix[i]+"diffuse.png", std::ios::binary | std::ios::trunc);
    image::ImageEncoder::Encode(os, image::ImageEncoder::Format::PNG, dst.GetImageForFace((image::Cubemap::Face)i));
    os.close();
  }

  //image::LinearImage sub = tmp.Subset(512, 512, 1024, 512);
  std::ofstream os("whole_diffuse.png", std::ios::binary | std::ios::trunc);
  image::ImageEncoder::Encode(os, image::ImageEncoder::Format::PNG, whole_image);
  os.close();
  std::cout <<"output diffuse finished!";
}

int main(int argc, char** argv) {
//  if (argc != 1) {
//    return -1;
//  }
  std::string cmd = "prefilter";

  if (cmd == "lut") {
    OutputLUT();
    return 0;
  }

  std::string input_file("default.jpg");
  image::LinearImage tmp;
  image::Cubemap base = image::CubemapUtils::CreateCubemap(tmp, 1024);


  int w, h, n;
//  unsigned char* file_data = stbi_load(input_file.c_str(), &w, &h, &n, 3);
//  image::LinearImage linear_image = image::FromSRGBToLinear(w, h, 3 * w, file_data);
//  stbi_image_free(file_data);

  std::cout<<"start set face for base level cubemap"<<std::endl;
//  image::CubemapUtils::CrossToCubemap(base, linear_image);
  // 初始化cubemap
  for (int i = 0; i < 6; ++i) {
    std::string file_name = "0" + prefix[i] + input_file;
    unsigned char* file_data = stbi_load(file_name.c_str(), &w, &h, &n, 3);
    image::LinearImage linear_image = image::FromRGBToLinear(w, h, 3 * w, file_data);
    stbi_image_free(file_data);
    image::CubemapUtils::SetFaceFromSingleImage(base, (image::Cubemap::Face)i, linear_image);
  }

  std::cout<<"start make base level seamless"<<std::endl;
  std::vector<image::Cubemap> levels;
  levels.push_back(std::move(base));
  levels[0].MakeSeamless();


  std::cout<<"start generate mipmaps!"<<std::endl;
  image::CubemapUtils::GenerateMipmaps(levels);


  if (cmd == "prefilter") {
    std::cout<<"start generate prefilter!"<<std::endl;
    OutputPrefilter(5, 256, levels);
  } else if (cmd == "diffuse"){
    std::cout << "start generate diffuse irradiance!" << std::endl;
    OutputDiffuse(256, levels);
  }

  return 0;
}