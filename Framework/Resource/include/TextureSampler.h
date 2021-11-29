//
// Created by Glodxy on 2021/11/27.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TEXTURESAMPLER_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TEXTURESAMPLER_H_

#include "Backend/include/DriverEnum.h"
namespace our_graph {

class TextureSampler {
 public:
  using WrapMode = SamplerWrapMode;
  using MinFiler = SamplerMinFilter;
  using MagFiler = SamplerMagFilter;
  using CompareMode = SamplerCompareMode;
  using CompareFunc = SamplerCompareFunc;


  TextureSampler() noexcept = default;
  TextureSampler(const TextureSampler& ) noexcept = default;
  TextureSampler& operator=(const TextureSampler&) noexcept = default;

  explicit TextureSampler(MagFiler min_mag, WrapMode str = WrapMode::CLAMP_TO_EDGE) noexcept {
    sampler_params_.filterMin = MinFiler(min_mag);
    sampler_params_.filterMag = min_mag;
    sampler_params_.wrapS = str;
    sampler_params_.wrapT = str;
    sampler_params_.wrapR = str;
  }

  TextureSampler(MinFiler min, MagFiler mag, WrapMode str = WrapMode::CLAMP_TO_EDGE) noexcept {
    sampler_params_.filterMin = min;
    sampler_params_.filterMag = mag;
    sampler_params_.wrapS = str;
    sampler_params_.wrapT = str;
    sampler_params_.wrapR = str;
  }

  TextureSampler(MinFiler min, MagFiler mag, WrapMode s, WrapMode t, WrapMode r) noexcept {
    sampler_params_.filterMin = min;
    sampler_params_.filterMag = mag;
    sampler_params_.wrapS = s;
    sampler_params_.wrapR = r;
    sampler_params_.wrapT = t;
  }

  explicit TextureSampler(CompareMode mode, CompareFunc func = CompareFunc::LE) {
    sampler_params_.compareMode = mode;
    sampler_params_.compareFunc = func;
  }

  void SetMinFilter(MinFiler v) noexcept {
    sampler_params_.filterMin = v;
  }

  void SetMagFilter(MagFiler v) noexcept {
    sampler_params_.filterMag = v;
  }

  void SetWrapModeS(WrapMode s) noexcept {
    sampler_params_.wrapS = s;
  }

  void SetWrapModeT(WrapMode t) noexcept {
    sampler_params_.wrapT = t;
  }

  void SetWrapModeR(WrapMode r) noexcept {
    sampler_params_.wrapR = r;
  }

  /**
   * 设置各项异性
   * 各项异性过滤的层数最大为7，默认为0
   * */
  void SetAnisotropy(float anisotropy) noexcept {
    const int log2 = ilogbf(anisotropy > 0 ? anisotropy : -anisotropy);
    sampler_params_.anisotropyLog2 = uint8_t(log2 < 7 ? log2 : 7);
  }

  void SetCompareMode(CompareMode mode, CompareFunc func = CompareFunc::LE) noexcept {
    sampler_params_.compareMode = mode;
    sampler_params_.compareFunc = func;
  }

  MinFiler GetMinFilter() const noexcept {
    return sampler_params_.filterMin;
  }

  MagFiler GetMagFilter() const noexcept {
    return sampler_params_.filterMag;
  }

  WrapMode GetWrapModeS() const noexcept {
    return sampler_params_.wrapS;
  }

  WrapMode GetWrapModeT() const noexcept {
    return sampler_params_.wrapT;
  }

  WrapMode GetWrapModeR() const noexcept {
    return sampler_params_.wrapR;
  }

  /**
   * 获取各向异性过滤的数值
   * sampler_params_内存储的为层数
   * */
  float GetAnisotropy() const noexcept {
    return float(1u << sampler_params_.anisotropyLog2);
  }

  CompareFunc GetCompareFunc() const noexcept {
    return sampler_params_.compareFunc;
  }

  CompareMode GetCompareMode() const noexcept {
    return sampler_params_.compareMode;
  }
 private:
  SamplerParams sampler_params_;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_TEXTURESAMPLER_H_
