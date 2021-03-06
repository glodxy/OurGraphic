//
// Created by Glodxy on 2022/1/3.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_SAMPLERSTRUCT_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_SAMPLERSTRUCT_H_

namespace our_graph {

struct PerViewSamplerBlock {
  enum {
    DiffuseIrradiance = 0, // IBL 漫反射
    SpecularPrefilter, // IBL 高光反射
    BrdfLut, // IBL brdf
    kSamplerCount, // 采样器数量
  };
};

}
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_SAMPLERSTRUCT_H_
