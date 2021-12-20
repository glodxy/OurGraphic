//
// Created by Glodxy on 2021/12/9.
//

#include "SamplerBlockGenerator.h"

namespace our_graph {

const SamplerBlock & SamplerBlockGenerator::GenerateSamplerBlock(BindingPoints binding_points, uint32_t key) {
  switch (binding_points) {
    case BindingPoints::PER_VIEW : {
      return GeneratePerViewBlock(key);
    }
  }
}

}  // namespace our_graph