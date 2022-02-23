//
// Created by Glodxy on 2022/2/14.
//

#include "CubemapIBL.h"
#include <iostream>
#include "CubemapUtils.h"
#include <random>
#include "Utils/Math/TransformUtils.h"

namespace our_graph::image {


static math::Vec3 HemisphereImportanceSampleDGGX(math::Vec2 u, float a) {
  // 获取角的大小
  const float phi = 2.0 * float(math::PI) * u.x;

  const float cos_theta2 = (1 - u.y) /(1 + (a + 1) * ((a - 1) * u.y));
  const float cos_theta = std::sqrt(cos_theta2);
  const float sin_theta = std::sqrt(1 - cos_theta2);
  return {sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta};
}

/**
 * GGX的法线分布函数
 * */
static float DistributionGGX(float NoH, float roughness) {
  float a = roughness;
  float f = (a - 1) * ((a + 1) * (NoH * NoH)) + 1;
  return (a * a) / ((float) math::PI * f * f);
}

/**
 * 与高度相关的几何分布函数
 * */
static float Visibility(float NoV, float NoL, float a) {
  // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
  // Height-correlated GGX
  const float a2 = a * a;
  const float GGXL = NoV * std::sqrt((NoL - NoL * a2) * NoL + a2);
  const float GGXV = NoL * std::sqrt((NoV - NoV * a2) * NoV + a2);
  return 0.5f / (GGXV + GGXL);
}

/**
 * 对半圆进行采样
 * 以x-y为底面, 切线空间进行采样
 * 其中theta为
 *  _______
 * /  ｜  /\
 *|___｜/___|
 *     |
 *     ---theta
 * 即与法线的夹角
 *
 * phi为与x轴正向的夹角
 * */
static math::Vec3 HemisphereCosSample(math::Vec2 u) {
  // 计算角度
  const float phi = 2.0f * (float) math::PI * u.x;
  const float cos_theta2 = 1 - u.y;
  const float cos_theta = std::sqrt(cos_theta2);
  const float sin_theta = std::sqrt(1 - cos_theta2);
  return { sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta };
}


/**
 * brdf提取出来的D()F()V()函数
 * 计算得到该参数下的brdf值
 * */
static math::Vec2 DFV(float NoV, float roughness, size_t num_samples) {
  math::Vec2 r(0);
  // 得到视线方向
  const math::Vec3 V(std::sqrt(1 - NoV * NoV), 0, NoV);
  for (size_t i = 0; i < num_samples; i++) {
    const math::Vec2 u = CubemapUtils::Hammersley(uint32_t(i), 1.0f / num_samples);
    // 采样得到半程向量
    const math::Vec3 H = HemisphereImportanceSampleDGGX(u, roughness);
    // 通过半程向量计算得到光照方向
    const math::Vec3 L = 2 * dot(V, H) * H - V;
    const float VoH = glm::clamp(dot(V, H), 0.f, 1.f);
    const float NoL = glm::clamp(L.z, 0.f, 1.f);
    const float NoH = glm::clamp(H.z, 0.f, 1.f);
    if (NoL > 0) {
      const float v = Visibility(NoV, NoL, roughness) * NoL * (VoH / NoH);
      const float Fc = std::pow(1 - VoH, 5);
      r.x += v * (1.0f - Fc);
      r.y += v * Fc;
    }
  }
  return r * (4.0f / num_samples);
}

void CubemapIBL::DiffuseIrradiance(Cubemap &dst, const std::vector<Cubemap> &levels, size_t max_sample_nums) {
  const float num_samples = max_sample_nums;
  const float i_num_samples = 1.0f / num_samples;
  const size_t max_level = levels.size()-1;
  const float max_level_f = max_level;
  const Cubemap& base(levels[0]);
  const size_t dim0 = base.GetDimension();
  const float omega_p = (4.0f * (float) math::PI) / float(6 * dim0 * dim0);

  // 单个采样点的数据
  struct SampleCache {
    math::Vec3 L; // 光线，即采样的方向
    float lerp; // 两个level之间的比率
    uint8_t l0; // level 0
    uint8_t l1;
  };

  // 该数组用于缓存所有的采样点信息
  std::vector<SampleCache> caches;
  caches.reserve(num_samples);

  for (size_t sample_idx = 0; sample_idx < num_samples; ++ sample_idx) {
    const math::Vec2 u = CubemapUtils::Hammersley(sample_idx, i_num_samples);
    // 获取光线的方向，x-y为底面，
    const math::Vec3 L = HemisphereCosSample(u);
    // 该面的法线
    const math::Vec3 N = {0, 0, 1};
    const float NoL = glm::dot(N, L);

    if (NoL > 0) {
      float pdf = NoL * float(math::I_PI);

      constexpr float K = 4;
      const float omega_s = 1.0f / (num_samples * pdf);
      const float l = float(math::Log4(omega_s) - math::Log4(omega_p) + math::Log4(K));
      const float mip_level = glm::clamp(float(l), 0.0f, max_level_f);
      uint8_t l0 = uint8_t(mip_level);
      uint8_t l1 = uint8_t(std::min(max_level, size_t(l0) + 1));
      float lerp = mip_level - (float)l0;
      caches.push_back({L, lerp, l0, l1});
    }
  }
  std::cout<<"generate sample light finished"<<std::endl;
  /**
   * 构建每个面的处理函数
   * @param face:当前处理的面
   * @param data:当前面的起始数据
   * @param dim:要处理的texel数量
   * */
  auto ProcessFace = [&](Cubemap::Face face, size_t dim) {
    LinearImage& image(dst.GetImageForFace(face));
    // 按行处理
    for (size_t y = 0; y < dim; ++y) {
      Cubemap::Texel * data = static_cast<Cubemap::Texel*>((void*)image.GetPixel(y, 0));

      math::Mat3 R;
      const size_t sample_cnt = caches.size();
      // 处理该行的每一个texel
      for (size_t x = 0; x < dim; ++x, ++data) {
        // 获取该texel所对应的法线
        const math::Vec2 p(Cubemap::GetCenter(x, y));
        const math::Vec3 N(dst.GetDirectionFor(face, p.x, p.y));

        // 根据法线方向确认up方向
        // 对于top/bot两个面，up方向另取
        const math::Vec3 up = std::abs(N.z) < 0.999 ? math::Vec3(0, 1, 0) : math::Vec3(0, 0, -1);

        // 求出以N为z方向和up构成的坐标系对应的旋转矩阵
        //! 因为glm矩阵以列的方式存储，所以不需要转置便可以得到从该局部坐标系到全局坐标的变换
        R[0] = glm::normalize(glm::cross(up, N));
        R[1] = glm::cross(N, R[0]);
        R[2] = N;
        //R = glm::transpose(R);

        // 以该点为原点，应用之前所采样的半圆上的光线
        math::Vec3 Li = math::Vec3(0);
        for (size_t sample = 0; sample < sample_cnt; ++sample) {
          const SampleCache& e = caches[sample];
          // 应用变换矩阵，得到该切线空间下的光线在真实空间中的方向
          const math::Vec3 L(R * e.L);
          const Cubemap& cubemap0 = levels[e.l0];
          const Cubemap& cubemap1 = levels[e.l1];
          // 对env 纹理进行采样，得到该光线的结果
          const math::Vec3 tex = Cubemap::TrilinearFilterAt(cubemap0, cubemap1, e.lerp, L);
          Li += tex;
        }
        Cubemap::WriteAt(Cubemap::Texel(Li * i_num_samples), data);
      }
    }
  };

  const size_t target_dim = dst.GetDimension();
  for (size_t i = 0; i < 6; ++i) {
    std::cout<<" start process face:" <<i<<std::endl;
    ProcessFace((Cubemap::Face)i, target_dim);
  }
}

void CubemapIBL::RoughnessFilter(Cubemap &dst,
                                 const std::vector<Cubemap> &levels,
                                 float roughness,
                                 size_t max_num_samples,
                                 math::Vec3 mirror,
                                 bool prefilter) {
  const float num_samples = max_num_samples;
  const float i_num_samples = 1.0f / num_samples;
  const size_t max_level = levels.size() - 1;
  const float max_level_f = float(max_level);
  const Cubemap &base(levels[0]);
  const size_t dim0 = base.GetDimension();
  const float omega_p = (4.0f * (float) math::PI / float(6 * dim0 * dim0));

  // 粗糙度为0，完全光滑，直接取反射
  if (roughness == 0) {
    // 单行处理函数
    auto scanline = [&](size_t y, Cubemap::Face f, Cubemap::Texel *data, size_t dim) {
      const Cubemap &cubemap = levels[0];
      for (size_t x = 0; x < dim; ++x, ++data) {
        const math::Vec2 p(Cubemap::GetCenter(x, y));
        const math::Vec3 N(dst.GetDirectionFor(f, p.x, p.y) * mirror);
        Cubemap::WriteAt(cubemap.SampleAt(N), data);
      }
    };

    for (size_t f = 0; f < 6; ++f) {
      auto& image = dst.GetImageForFace((Cubemap::Face)f);
      size_t dim = dst.GetDimension();
      for (size_t y = 0; y < dim; ++y) {
        Cubemap::Texel *data = static_cast<Cubemap::Texel *>((void *) image.GetPixel(y, 0));
        // 处理单行
        scanline(y, (Cubemap::Face)f, data, dim);
      }
    }

    return;
  }

  struct CacheEntry {
    math::Vec3 L;
    float brdf_NoL;
    float lerp;
    uint8_t l0;
    uint8_t l1;
  };

  std::vector<CacheEntry> cache;
  cache.reserve(max_num_samples);

  // 预计算仅依赖采样的项
  float weight = 0;

  // 生成采样点
  for (size_t sample_index = 0; sample_index < max_num_samples; ++ sample_index) {
    // 获取半圆的分布
    const math::Vec2 u = CubemapUtils::Hammersley(uint32_t(sample_index), i_num_samples);

    // GGX重要性采样
    // 采样得到半程向量
    const math::Vec3 H = HemisphereImportanceSampleDGGX(u, roughness);

    const float NoH = H.z;
    const float NoH2 = H.z * H.z;
    const float NoL = 2 * NoH2 - 1;
    const math::Vec3 L(2 * NoH * H.x, 2 * NoH * H.y, NoL);

    // 代表光会对该面起作用
    if (NoL > 0) {
      // 获取法线的分布概率
      const float pdf = DistributionGGX(NoH, roughness) / 4;

      constexpr float K = 4;
      // 得到用于蒙特卡洛法的概率分母
      const float omega_s = 1 / (num_samples * pdf);
      // 1 + log4(omega_s/omega_p)
      const float l = float(math::Log4(omega_s) - math::Log4(omega_p) + math::Log4(K));

      const float mip_level = prefilter ? glm::clamp(float(l), .0f, max_level_f) : 0.0f;

      const float brdf_NoL = float(NoL);
      weight += brdf_NoL;

      const uint8_t l0 = uint8_t(mip_level);
      const uint8_t l1 = std::min(max_level, size_t(l0 + 1));
      float lerp = mip_level - (float)l0;

      cache.push_back({L, brdf_NoL, lerp, l0, l1});
    }
  }

  // 添加权重
  for (auto& entry : cache) {
    entry.brdf_NoL *= 1.0f / weight;
  }

  // 按照权重 升序排序
  std::sort(cache.begin(), cache.end(), [](const CacheEntry& l, const CacheEntry& r) {
    return l.brdf_NoL < r.brdf_NoL;
  });

  std::default_random_engine gen;
  std::uniform_real_distribution<float> distribution { -math::PI, math::PI};

  auto process = [&](size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
    math::Mat3 R;
    const size_t num_samples = cache.size();
    for (size_t x = 0; x < dim; ++data, ++x) {
      const math::Vec2 p(Cubemap::GetCenter(x, y));
      // 获取法线
      const math::Vec3 N(dst.GetDirectionFor(f, p.x, p.y) * mirror);

      const math::Vec3 up = std::abs(N.z) < 0.999f ? math::Vec3(0, 1, 0) : math::Vec3(0, 0, -1);
      R[0] = glm::normalize(glm::cross(up, N));
      R[1] = glm::cross(N, R[0]);
      R[2] = N;

      // 添加旋转扰动
      // note:该旋转发生在转换坐标系之前，是在切线空间的旋转，需要右乘
      R *= math::Mat3(math::TransformUtils::Rotate(math::Vec3(0, 0, 1), distribution(gen)));

      math::Vec3 Li(0);
      // 遍历所有采样点
      for (size_t sample = 0; sample < num_samples; ++sample) {
        const CacheEntry& e = cache[sample];
        // 计算得到采样的光线方向
        const math::Vec3 L(R * e.L);
        const Cubemap& base = levels[e.l0];
        const Cubemap& next = levels[e.l1];
        const math::Vec3 c0 = Cubemap::TrilinearFilterAt(base, next, e.lerp, L);
        Li += c0 * e.brdf_NoL;
      }

      Cubemap::WriteAt(Li, data);
    }
  };

  for (size_t f = 0; f < 6; ++f) {
    auto &image = dst.GetImageForFace((Cubemap::Face) f);
    size_t dim = dst.GetDimension();
    for (size_t y = 0; y < dim; ++y) {
      Cubemap::Texel *data = static_cast<Cubemap::Texel *>((void *) image.GetPixel(y, 0));
      process(y, (Cubemap::Face)f, data, dim);
    }
  }
}

void CubemapIBL::LUT(LinearImage &dst) {
  auto dfv_function = DFV;
  const float width = (float)dst.GetWidth();
  for (size_t y = 0; y < dst.GetHeight(); ++y) {
    Cubemap::Texel * data = static_cast<Cubemap::Texel*>((void*)dst.GetPixel(y, 0));

    const float h = (float)dst.GetHeight();
    const float coord = (h - y) / h;
    // roughness直接用y映射
    const float roughness = coord;

    for (size_t x = 0; x < dst.GetWidth(); ++x, ++data) {
      // const float NoV = float(x) / (width-1);
      const float NoV = glm::clamp((x + 0.5f) / width, .0f, 1.f);
      math::Vec3 r = { dfv_function(NoV, roughness, 1024), 0 };
      *data = r;
    }
  }
}

}