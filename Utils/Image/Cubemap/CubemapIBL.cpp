//
// Created by Glodxy on 2022/2/14.
//

#include "CubemapIBL.h"

#include "CubemapUtils.h"

namespace our_graph::image {


/**
 * 对半圆进行采样
 * 以x-y为底面, todo:确认此处与实际使用坐标不对应是否有影响
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
        const math::Vec3 up = std::abs(N.z) < 0.999 ? math::Vec3(0, 0, 1) : math::Vec3(1, 0, 0);

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
    ProcessFace((Cubemap::Face)i, target_dim);
  }
}

}