//
// Created by Glodxy on 2021/12/5.
//
#include "include/Material.h"
#include "ResourceAllocator.h"
#include "Material/SamplerBindingMap.h"
#include "Framework/include/GlobalEnum.h"
#include "Framework/Resource/Material/MaterialParser.h"
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
                            SamplerBlock& block,
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
  parser->GetMaterialProperties(material_properties_);
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

  
}


}  // namespace our_graph
