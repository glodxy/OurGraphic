//
// Created by Glodxy on 2021/12/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_MATERIALINFO_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_MATERIALINFO_H_
#include "Framework/Backend/include/DriverEnum.h"
#include "Framework/Resource/Material/SamplerBlock.h"
#include "Framework/Resource/Material/UniformBlock.h"
#include "Framework/Resource/include_internal/MaterialEnum.h"
namespace our_graph {
struct MaterialInfo {
  // todo:是否具有双面性质
  bool has_double_sided_capability;
  // todo:是否具有透明物体的阴影
  bool has_transparent_shadow;
  // todo:是否有自定义表面着色
  bool has_custom_surface_shading;
  RefractionMode refraction_mode;
  RefractionType refraction_type;
  // 有使用哪些attribute
  AttributeBitset required_attributes;
  // 混合模式
  BlendingMode blending_mode;

};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_MATERIALINFO_H_
