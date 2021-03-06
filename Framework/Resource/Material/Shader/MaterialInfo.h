//
// Created by Glodxy on 2021/12/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_MATERIALINFO_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_MATERIALINFO_H_
#include "Framework/Backend/include/DriverEnum.h"
#include "Framework/Resource/Material/SamplerBlock.h"
#include "Framework/Resource/Material/UniformBlock.h"
#include "Framework/Resource/Material/SamplerBindingMap.h"
#include "Framework/Resource/include_internal/MaterialEnum.h"
#include <list>
namespace our_graph {
struct ParamValuePair {
  std::string key;
  UniformType type;
  std::string value;
};

// 该结构表示了在material的shader之间传递的变量
struct Variant {
  std::string name;
  std::string type;
  uint32_t size;
};
// 描述了一个subpass
struct SubpassInfo {
  uint8_t subpass_input;
  uint8_t subpass_output;
  // 该pass所使用的着色器
  std::string vertex_shader;
  std::string frag_shader;
};
using VariantList = Variant[MATERIAL_VARIABLES_COUNT];
using PropertyList = std::vector<MaterialProperty::Property>;
using ParamValueList = std::vector<ParamValuePair>;
/**
 * 该结构体存储了创建一个材质的shader所需要的全部信息
 * 该部分也只会被ShaderGenerator使用
 * */
struct MaterialInfo {
  // 是否写入颜色
  bool color_write;
  // 裁剪模式
  CullingMode culling_mode;
  // 是否有自定义的深度shader
  bool has_custom_depth_shader;
  // 是否启用深度测试
  bool enable_depth_test;
  // 是否深度写入
  bool depth_write;
  // 遮掩的阈值
  float mask_threshold;
  // material的类型(surface/post process)
  MaterialDomain domain;
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
  // 延迟光照的混合模式
  BlendingMode post_lighting_blending_mode;
  // 着色模式
  ShadingModel shading_model;
  UniformBlock uniform_block;
  SamplerBlock sampler_block;
  SamplerBindingMap sampler_binding_map;

  RenderPath render_path;

  // 该项为shader之间传递的变量
  VariantList variant_list;

  std::vector<SubpassInfo> pass_list;

  ParamValueList default_param_value;

  // property, 即材质使用了的内置变量
  PropertyList property_list;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADER_MATERIALINFO_H_
