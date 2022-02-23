//
// Created by Glodxy on 2021/12/9.
//

#include "Framework/Resource/Material/SamplerBlockGenerator.h"

namespace our_graph {

const SamplerBlock & SamplerBlockGenerator::GeneratePerViewBlock() {
  auto buildSampler = []() {
    SamplerBlock::Builder builder;
    builder.Name("FrameSampler");

    //builder.Add("sky", SamplerType::SAMPLER_CUBEMAP, SamplerFormat::FLOAT);
    builder.Add("diffuseIrradiance", SamplerType::SAMPLER_CUBEMAP, SamplerFormat::FLOAT);
    builder.Add("specularPrefilter", SamplerType::SAMPLER_CUBEMAP, SamplerFormat::FLOAT);
    builder.Add("brdfLut", SamplerType::SAMPLER_2D, SamplerFormat::FLOAT);
    return builder.Build();
  };
  static SamplerBlock sampler_block = buildSampler();

  return sampler_block;
}


}  // namespace our_graph