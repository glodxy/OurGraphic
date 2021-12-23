//
// Created by Glodxy on 2021/12/5.
//
#include "include/Material.h"
#include "ResourceAllocator.h"
#include "Material/SamplerBindingMap.h"
#include "Framework/include/GlobalEnum.h"
#include "Framework/Resource/Material/MaterialParser.h"
#include "Framework/Resource/Material/ShaderBuilder.h"
#include "Framework/Resource/Material/SamplerBlockGenerator.h"
namespace our_graph {
static uint32_t GenerateMaterialID() {
  static uint32_t id = 1;
  return ++id;
}

struct Material::Details {
  const void* data = nullptr;
  size_t size = 0;
  MaterialParser* parser = nullptr;
  bool default_material = false;
  Driver* driver = nullptr;
};

using BuilderType = Material;
BuilderType::Builder::Builder(Driver* driver) noexcept {
  impl_->driver = driver;
}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

Material::Builder &Material::Builder::Data(const void *data, size_t size) {
  impl_->data = data;
  impl_->size = size;
  return *this;
}

Material *Material::Builder::Build() {
  // todo:创建material parser
  MaterialParser* material_parser = nullptr;
  // 与filament不同，只支持glsl，故不检查着色器语言
  impl_->parser = material_parser;
  return ResourceAllocator::Get().CreateMaterial(*this);
}

/*-------------------Material--------------*/
/**
 * 将block中的所有sampler生成sasmpler group
 * @note: 因为每个block并没有限制绑定位置，
 * 所以需手动传入其相应的binding_point
 * */
static void AddSamplerGroup(Program& program, uint8_t binding_point,
                            const SamplerBlock& block,
                            SamplerBindingMap const& map) {
  // 获取sampler的数量
  const size_t sampler_cnt = block.GetSize();
  if (sampler_cnt) {
    // 存在sampler的时候，进行绑定
    std::vector<Program::Sampler> samplers(sampler_cnt);
    auto const& list = block.GetSamplerInfoList();
    for (size_t i = 0; i < sampler_cnt; ++i) {
      // 根据sampler block以及sampler的名称来生成uniform名称
      std::string uniform_name = SamplerBlock::GetUniformName(
          block.GetName(), list[i].name);
      uint8_t binding = 0;
      bool suc = map.GetSamplerBinding(binding_point, (uint8_t)i, &binding);
      if (!suc) {
        LOG_ERROR("Material", "Add Sampler Group Failed! sampler[{}] with binding point[{}] "
                              "cannot get global binding!"
                              "block name:{}, sampler name:{}",
                              i, binding_point, block.GetName(),
                              list[i].name);
        assert(suc);
        return;
      }
      // 材质的采样器必须绑定纹理
      const bool strict = (binding_point == BindingPoints::PER_MATERIAL_INSTANCE);
      samplers[i] = {std::move(uniform_name), binding, strict};
    }
    program.SetSamplerGroup(binding_point, samplers.data(), samplers.size());
  }
}

Material::Material(const Builder &builder) :
  driver_(builder->driver),
  material_id_(GenerateMaterialID()) {
  MaterialParser* parser = builder->parser;
  material_parser_ = parser;

  // 获取材质名
  bool suc = parser->GetName(name_);
  if (!suc) {
    LOG_ERROR("Material", "Parse Name Failed!");
    assert(suc);
    return;
  }

  // 获取uniform buffer
  suc = parser->GetUniformBlock(uniform_block_);
  if (!suc) {
    LOG_ERROR("Material", "Parse UniformBlock Failed!");
    assert(suc);
    return;
  }

  // 获取sampler buffer
  suc = parser->GetSamplerBlock(sampler_block_);
  if (!suc) {
    LOG_ERROR("Material", "Parse SamplerBlock Failed!");
    assert(suc);
    return;
  }

  // todo:subpass

  sampler_binding_map_.Init(&sampler_block_);

  parser->GetShadingModel(shading_);
  parser->GetBlendingModel(blending_mode_);
  parser->GetMaterialDomain(material_domain_);
  parser->GetRequiredAttributes(required_attributes_);
  parser->GetRefractionMode(refraction_mode_);
  parser->GetRefractionType(refraction_type_);

  if (blending_mode_ == BlendingMode::MASKED) {
    parser->GetMaskThreshold(mask_threshold_);
  }

  // fade模式只影响着色，实际的渲染过程仍然按照透明进行
  if (blending_mode_ == BlendingMode::FADE) {
    render_blending_mode_ = BlendingMode::TRANSPARENT;
  } else {
    render_blending_mode_ = blending_mode_;
  }


  using BlendFunction = RasterState::BlendFunction;
  using DepthFunc = RasterState::DepthFunc;
  switch (blending_mode_) {
    case BlendingMode::OPAQUE: {
      // 该模式会将rgba完全覆盖
      raster_state_.blendFunctionDstRGB = BlendFunction::ZERO;
      raster_state_.blendFunctionDstAlpha = BlendFunction::ZERO;
      raster_state_.blendFunctionSrcRGB = BlendFunction::ONE;
      raster_state_.blendFunctionSrcAlpha = BlendFunction::ONE;
      raster_state_.depthWrite = true;
      break;
    }
    case BlendingMode::MASKED: {
      // 该模式只保留rgb， 不改变alpha值
      raster_state_.blendFunctionSrcRGB = BlendFunction::ONE;
      raster_state_.blendFunctionSrcAlpha = BlendFunction::ZERO;
      raster_state_.blendFunctionDstRGB = BlendFunction::ZERO;
      raster_state_.blendFunctionDstAlpha = BlendFunction::ONE;
      raster_state_.depthWrite = true;
      break;
    }
    case BlendingMode::FADE:
    case BlendingMode::TRANSPARENT: {
      // 透明渲染会使用alpha值，并禁用深度
      raster_state_.blendFunctionSrcRGB = BlendFunction::ONE;
      raster_state_.blendFunctionSrcAlpha = BlendFunction::ONE;
      raster_state_.blendFunctionDstRGB = BlendFunction::ONE_MINUS_SRC_ALPHA;
      raster_state_.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
      raster_state_.depthWrite = false;
      break;
    }
    case BlendingMode::ADD: {
      // 该模式直接所有相加
      raster_state_.blendFunctionSrcRGB = BlendFunction::ONE;
      raster_state_.blendFunctionSrcAlpha = BlendFunction::ONE;
      raster_state_.blendFunctionDstRGB = BlendFunction::ONE;
      raster_state_.blendFunctionDstAlpha = BlendFunction::ONE;
      raster_state_.depthWrite = false;
      break;
    }
    case BlendingMode::MULTIPLY: {
      // 该模式为颜色值相乘
      raster_state_.blendFunctionSrcRGB = BlendFunction::ZERO;
      raster_state_.blendFunctionSrcAlpha = BlendFunction::ZERO;
      raster_state_.blendFunctionDstRGB = BlendFunction::SRC_COLOR;
      raster_state_.blendFunctionDstAlpha = BlendFunction::SRC_COLOR;
      raster_state_.depthWrite = false;
      break;
    }
    case BlendingMode::SCREEN: {
      raster_state_.blendFunctionSrcRGB = BlendFunction::ONE;
      raster_state_.blendFunctionSrcAlpha = BlendFunction::ONE;
      raster_state_.blendFunctionDstRGB = BlendFunction::ONE_MINUS_SRC_COLOR;
      raster_state_.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_COLOR;
      raster_state_.depthWrite = false;
      break;
    }
  }
  parser->GetCullingMode(culling_mode_);

  bool depth_test = false;
  parser->GetDepthTest(depth_test);

  bool depth_write = true;
  if (parser->GetDepthWrite(depth_write)) {
    raster_state_.depthWrite = depth_write;
  }

  bool double_sided = false;
  if (parser->GetDoubleSided(double_sided)) {
    double_sided_capability_ = true;
    double_sided_ = double_sided;
    raster_state_.culling = double_sided_ ? CullingMode::NONE : culling_mode_;
  } else {
    raster_state_.culling = culling_mode_;
  }

  parser->GetTransparencyMode(transparency_mode_);
  parser->GetCustomDepthShaderSet(has_custom_depth_shader_);
  is_default_material_ = builder->default_material;

  // todo:对于默认材质与自定义深度shader，需要进行shader的cahce

  bool color_write = false;
  if (parser->GetColorWrite(color_write)) {
    raster_state_.colorWrite = color_write;
  }
  raster_state_.depthFunc = depth_test ? DepthFunc::LE : DepthFunc ::A;
  raster_state_.alphaToCoverage = blending_mode_ == BlendingMode::MASKED;

  default_instance_.InitDefaultInstance(driver_, this);
}

void Material::Destroy() {
  delete material_parser_;
  default_instance_.Destroy();
}

bool Material::HasParameter(const std::string &name) const noexcept {
  // todo: subpass info
  return uniform_block_.HasUniform(name) ||
          sampler_block_.HasSampler(name);
}

MaterialInstance * Material::CreateInstance(const std::string &name) {
  return MaterialInstance::Duplicate(&default_instance_, name);
}

bool Material::IsSampler(const std::string &name) const noexcept {
  return sampler_block_.HasSampler(name);
}

/*--------------------Shader-----------------------*/
// todo:生成shader相关数据
ShaderHandle Material::BuildShader(uint8_t key) const noexcept {
  switch (GetMaterialDomain()) {
    case MaterialDomain::SURFACE:{
      return BuildSurfaceShader(key);
    }
    case MaterialDomain::POST_PROCESS: {
      return BuildPostProcessShader(key);
    }
  }
}

ShaderHandle Material::BuildPostProcessShader(uint8_t key) const noexcept {
  Program shader = GetProgramByKey(key, key, key);
  AddSamplerGroup(shader, BindingPoints::PER_MATERIAL_INSTANCE, sampler_block_, sampler_binding_map_);
  return CreateAndCacheShader(std::move(shader), key);
}

ShaderHandle Material::BuildSurfaceShader(uint8_t key) const noexcept {
  // todo:filter key
  uint32_t vertex_key;
  uint32_t frag_key;

  Program shader = GetProgramByKey(key, vertex_key, frag_key);
  // 设置属性
  // 1.设置per view会使用的sampler
  AddSamplerGroup(shader, BindingPoints::PER_VIEW,
                  SamplerBlockGenerator::GetSamplerBlock(BindingPoints::PER_VIEW, key),
                  sampler_binding_map_);
  AddSamplerGroup(shader, BindingPoints::PER_MATERIAL_INSTANCE,
                  sampler_block_, sampler_binding_map_);
  return CreateAndCacheShader(std::move(shader), key);
}

Program Material::GetProgramByKey(uint32_t key, uint32_t vertex_key, uint32_t frag_key) const noexcept {
  // vertex
  ShaderBuilder vs_shader;
  if (!material_parser_->GetShader(vs_shader, vertex_key, ShaderType::VERTEX)) {
    LOG_ERROR("Material", "Mat[{}] Get Vertex Shader Failed!",
              GetName());
    exit(-1);
  }
  // pixel
  ShaderBuilder fs_shader;
  if (!material_parser_->GetShader(fs_shader, frag_key, ShaderType::FRAGMENT)) {
    LOG_ERROR("Material", "Mat[{}] Get Fragment Shader Failed!",
              GetName());
    exit(-1);
  }

  Program shader;
  shader.Diagnostics(GetName(), key)
      .WithVertexShader(vs_shader.GetData(), vs_shader.GetSize())
      .WithFragmentShader(fs_shader.GetData(), fs_shader.GetSize());
  return shader;
}

ShaderHandle Material::CreateAndCacheShader(Program &&p, uint32_t key) const noexcept {
  auto handle = driver_->CreateShader(std::move(p));
  cache_programs_[key] = handle;
  return handle;
}
/*--------------------------------------------------*/

void Material::GetParameters(std::vector<ParameterInfo> &parameters, size_t count) {
  count = std::min(count, GetParameterCount());
  // 1. 获取uniform
  const auto& uniform_list = uniform_block_.GetUniformInfoList();
  size_t i = 0;
  size_t uniform_cnt = std::min(count, uniform_list.size());
  for (; i < uniform_cnt; ++i) {
    const auto& uniform = uniform_list[i];
    ParameterInfo info {
      .name = uniform.name,
      .count = uniform.size,
      .type = uniform.type,
      .is_sampler = false,
      .is_subpass = false
    };
    parameters.push_back(info);
  }

  // 2. sampler
  const auto& sampler_list = sampler_block_.GetSamplerInfoList();
  size_t sampler_cnt = sampler_list.size();
  for (size_t j = 0; i < count && j < sampler_cnt; ++j) {
    const auto& sampler = sampler_list[i];
    ParameterInfo info {
      .name = sampler.name,
      .count = 1,
      .sampler_type = sampler.type,
      .is_sampler = true,
       .is_subpass = false
    };
    parameters.push_back(info);
  }

  // todo:subpass
}


}  // namespace our_graph
