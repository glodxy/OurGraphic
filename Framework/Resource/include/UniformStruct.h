//
// Created by Glodxy on 2022/1/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMSTRUCT_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMSTRUCT_H_
#include <string>
#include "Utils/Math/Math.h"

// 该宏用于找到结构体中的成员相对首位的偏移量
#define UNIFORM_MEMBER_OFFSET(s, m) ((size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
namespace our_graph {
/**
 * 该部分的结构体仅用于标识数据的构成
 * */
static const char* kPerViewUniformName = "FrameUniform";
static const char* kPerRenderableUniformName = "ObjectUniform";
static const char* kLightUnifromName = "LightUniform";
// 该uniform为每帧使用的uniform
struct PerViewUniformBlock {
  math::Mat4 viewFromWorldMat; // v matrix
  math::Mat4 worldFromViewMat;
  math::Mat4 clipFromViewMat; // p matrix
  math::Mat4 viewFromClipMat;
  math::Mat4 clipFromWorldMat;
  math::Mat4 worldFromClipMat;

  math::Vec4 resolution; // width height 1/width 1/height
  math::Vec3 cameraPosition; // world space下的相机坐标

  uint32_t time; // 以s为单位, 4byte
  uint32_t lightCount; // 光源数量, 4byte
};

// 每个渲染体都有的uniform
struct PerRenderableUniformBlock {
  math::Mat4 worldFromModelMat;
  math::Mat3 worldFromModelNormalMat; // 法线的变换矩阵
};

// 光照的uniform
// 控制为Mat4
struct LightUniformBlock {
  math::Vec3 position;
  float falloff; // 1/falloff
  math::Vec4 color;
  math::Vec3 direction;
  float intensity;
  math::Vec4 padding;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_UNIFORMSTRUCT_H_
