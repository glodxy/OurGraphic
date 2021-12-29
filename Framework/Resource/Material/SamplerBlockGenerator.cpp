//
// Created by Glodxy on 2021/12/9.
//

#include "Framework/Resource/Material/SamplerBlockGenerator.h"
namespace our_graph {

const SamplerBlock* SamplerBlockGenerator::GenerateSamplerBlock(BindingPoints binding_points, uint32_t key) noexcept {
  switch (binding_points) {
    case BindingPoints::PER_VIEW : {
      return &GeneratePerViewBlock(key);
    }
  }
  return nullptr;
}

const SamplerBlock & SamplerBlockGenerator::GeneratePerViewBlock(uint32_t key) {
  static SamplerBlock block;
  return block;
}

}  // namespace our_graph