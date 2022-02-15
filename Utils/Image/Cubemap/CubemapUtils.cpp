//
// Created by Glodxy on 2022/2/14.
//

#include "CubemapUtils.h"

namespace our_graph::image {
math::Vec2 CubemapUtils::Hammersley(uint32_t i, float iN) {
  constexpr float tof = 0.5f / 0x80000000U;
  uint32_t bits = i;
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return { i * iN, bits * tof };
}
}  // namespace our_graph::image