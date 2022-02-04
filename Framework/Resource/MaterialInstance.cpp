//
// Created by Glodxy on 2021/12/7.
//

#include "include/MaterialInstance.h"
#include "include/Material.h"
#include "Utils/Math/Math.h"
#include "Framework/Resource/ResourceAllocator.h"
#include "include_internal/UniformTypeTraits.h"
namespace our_graph {

template<size_t S>
void MaterialInstance::SetParameterUntyped(const std::string& name,
                                      const void* value) noexcept {
  int64_t offset = material_->GetUniformBlock().GetUniformOffset(name, 0);
  if (offset >= 0) {
    // 当offset 大于等于0时，说明是正常的offset
    uniform_buffer_.SetUniformUntyped<S>(size_t(offset), value);
  }
}

template<size_t S>
void MaterialInstance::SetParameterUntyped(const std::string &name,
                                           const void *value,
                                           size_t cnt) noexcept {
  int64_t offset = material_->GetUniformBlock().GetUniformOffset(name, 0);
  if (offset >= 0) {
    // 当offset 大于等于0时，说明是正常的offset
    uniform_buffer_.SetUniformArrayUntyped<S>(size_t(offset), value, cnt);
  }
}

/*------------SetParameter-------------------*/
template<typename T, typename>
void MaterialInstance::SetParameter(const std::string &name, const T &value) {
  SetParameterUntyped<sizeof(T)>(name, &value);
}

// 对于bool 需要转为uint以维持数据4byte的大小统一
template<>
void MaterialInstance::SetParameter(const std::string &name, const bool &value) {
  SetParameter(name, (uint32_t)value);
}

// 此处需要显式声明所有支持类型的模板函数
template void MaterialInstance::SetParameter<float>(const std::string &name, const float &value);
template void MaterialInstance::SetParameter<math::Vec2>(const std::string &name, const math::Vec2 &value);
template void MaterialInstance::SetParameter<math::Vec3>(const std::string &name, const math::Vec3 &value);
template void MaterialInstance::SetParameter<math::Vec4>(const std::string &name, const math::Vec4 &value);
template void MaterialInstance::SetParameter<int32_t>(const std::string &name, const int32_t &value);
template void MaterialInstance::SetParameter<math::Vec2i>(const std::string &name, const math::Vec2i &value);
template void MaterialInstance::SetParameter<math::Vec3i>(const std::string &name, const math::Vec3i &value);
template void MaterialInstance::SetParameter<math::Vec4i>(const std::string &name, const math::Vec4i &value);
template void MaterialInstance::SetParameter<uint32_t>(const std::string &name, const uint32_t &value);
template void MaterialInstance::SetParameter<math::Vec2u>(const std::string &name, const math::Vec2u &value);
template void MaterialInstance::SetParameter<math::Vec3u>(const std::string &name, const math::Vec3u &value);
template void MaterialInstance::SetParameter<math::Vec4u>(const std::string &name, const math::Vec4u &value);
template void MaterialInstance::SetParameter<math::Mat3>(const std::string &name, const math::Mat3 &value);
template void MaterialInstance::SetParameter<math::Mat4>(const std::string &name, const math::Mat4 &value);

/*-------------------------------------*/

/*------------SetParameterArray------------*/
template<typename T, typename>
void MaterialInstance::SetParameter(const std::string &name, const T *value, size_t size) {
  SetParameterUntyped<sizeof(T)>(name, value, size);
}

// 此处需要显式声明所有支持类型的模板函数
template void MaterialInstance::SetParameter<float>(const std::string &name, const float *value, size_t size);
template void MaterialInstance::SetParameter<math::Vec2>(const std::string &name, const math::Vec2 *value, size_t size);
template void MaterialInstance::SetParameter<math::Vec3>(const std::string &name, const math::Vec3 *value, size_t size);
template void MaterialInstance::SetParameter<math::Vec4>(const std::string &name, const math::Vec4 *value, size_t size);
template void MaterialInstance::SetParameter<int32_t>(const std::string &name, const int32_t *value, size_t size);
template void MaterialInstance::SetParameter<math::Vec2i>(const std::string &name, const math::Vec2i *value, size_t size);
template void MaterialInstance::SetParameter<math::Vec3i>(const std::string &name, const math::Vec3i *value, size_t size);
template void MaterialInstance::SetParameter<math::Vec4i>(const std::string &name, const math::Vec4i *value, size_t size);
template void MaterialInstance::SetParameter<uint32_t>(const std::string &name, const uint32_t *value, size_t size);
template void MaterialInstance::SetParameter<math::Vec2u>(const std::string &name, const math::Vec2u *value, size_t size);
template void MaterialInstance::SetParameter<math::Vec3u>(const std::string &name, const math::Vec3u *value, size_t size);
template void MaterialInstance::SetParameter<math::Vec4u>(const std::string &name, const math::Vec4u *value, size_t size);
template void MaterialInstance::SetParameter<math::Mat3>(const std::string &name, const math::Mat3 *value, size_t size);
template void MaterialInstance::SetParameter<math::Mat4>(const std::string &name, const math::Mat4 *value, size_t size);

/*----------------------------------------*/
/*-------------SetTexture-----------------*/
void MaterialInstance::SetParameter(const std::string &name, TextureHandle handle, SamplerParams param) {
  size_t index = material_->GetSamplerBlock().GetSamplerInfo(name)->offset;
  sampler_group_.SetSampler(index, {handle, param});
}

void MaterialInstance::SetParameter(const std::string &name, const Texture *texture, const TextureSampler &sampler) {
  size_t index = material_->GetSamplerBlock().GetSamplerInfo(name)->offset;
  sampler_group_.SetSampler(index, {texture->GetHandle(), sampler.GetSamplerParams()});
}
/*----------------------------------------*/
/*-------------SetAttribute---------------*/
void MaterialInstance::SetMaskThreshold(float threshold) noexcept {
  SetParameter("_mask_threshold", threshold);
}

void MaterialInstance::SetDoubleSided(bool double_sided) noexcept {
  if (!material_->HasDoubleSidedCapability()) {
    LOG_ERROR("MaterialInstance", "Mat[{}] not support double sided!",
              GetName());
    return;
  }
  SetParameter("_double_sided", double_sided);
  if (double_sided) {
    SetCullingMode(CullingMode::NONE);
  }
}

void MaterialInstance::SetDepthCulling(bool enable) noexcept {
  depth_func_ = enable ? RasterState::DepthFunc::LE : RasterState::DepthFunc::A;
}

std::string MaterialInstance::GetName() const noexcept {
  if (name_.empty()) {
    return material_->GetName();
  }
  return name_;
}

MaterialInstance::MaterialInstance(const MaterialInstance *src,
                                   const std::string &name) :
    driver_(src->driver_),
    material_(src->material_),
    culling_mode_(src->culling_mode_),
    color_write_(src->color_write_),
    depth_write_(src->depth_write_),
    depth_func_(src->depth_func_),
    name_(name.empty() ? src->name_: name) {
  const Material * const material = src->GetMaterial();

  // 创建uniform buffer的实例
  if (!material->GetUniformBlock().IsEmpty()) {
    uniform_buffer_.SetUniforms(src->GetUniformBuffer());
    uniform_handle_ = driver_->CreateBufferObject(uniform_buffer_.GetSize(),
                                                 BufferObjectBinding::UNIFORM,
                                                 BufferUsage::DYNAMIC);
  }
  // 创建sampler
  if (!material->GetSamplerBlock().IsEmpty()) {
    sampler_group_.SetSamplers(src->GetSamplerGroup());
    sampler_handle_ = driver_->CreateSamplerGroup(sampler_group_.GetSize());
  }

  material_sorting_key_ = (((material_->GetId()) << 32) | material_->GenerateMaterialInstanceId());
}


MaterialInstance::~MaterialInstance() noexcept {
  // todo
}

MaterialInstance * MaterialInstance::Duplicate(
    const MaterialInstance *src,
    const std::string &name) noexcept {
  return ResourceAllocator::Get().CreateMaterialInstance(src, name);
}

void MaterialInstance::SetDefaultValue(const std::string &name, UniformType type, const std::string &value) {
  switch (type) {
    case UniformType::INT: {
      SetParameter(name, ParseFromString<UniformType::INT>(value));
      break;
    }
    case UniformType::INT2: {
      SetParameter(name, ParseFromString<UniformType::INT2>(value));
      break;
    }
    case UniformType::INT3: {
      SetParameter(name, ParseFromString<UniformType::INT3>(value));
      break;
    }
    case UniformType::INT4: {
      SetParameter(name, ParseFromString<UniformType::INT4>(value));
      break;
    }

    case UniformType::UINT: {
      SetParameter(name, ParseFromString<UniformType::UINT>(value));
      break;
    }
    case UniformType::UINT2: {
      SetParameter(name, ParseFromString<UniformType::UINT2>(value));
      break;
    }
    case UniformType::UINT3: {
      SetParameter(name, ParseFromString<UniformType::UINT3>(value));
      break;
    }
    case UniformType::UINT4: {
      SetParameter(name, ParseFromString<UniformType::UINT4>(value));
      break;
    }

    case UniformType::FLOAT: {
      SetParameter(name, ParseFromString<UniformType::FLOAT>(value));
      break;
    }
    case UniformType::FLOAT2: {
      SetParameter(name, ParseFromString<UniformType::FLOAT2>(value));
      break;
    }
    case UniformType::FLOAT3: {
      SetParameter(name, ParseFromString<UniformType::FLOAT3>(value));
      break;
    }
    case UniformType::FLOAT4: {
      SetParameter(name, ParseFromString<UniformType::FLOAT4>(value));
      break;
    }

    case UniformType::BOOL: {
      SetParameter(name, ParseFromString<UniformType::BOOL>(value));
      break;
    }
    case UniformType::BOOL2: {
      SetParameter(name, ParseFromString<UniformType::BOOL2>(value));
      break;
    }
    case UniformType::BOOL3: {
      SetParameter(name, ParseFromString<UniformType::BOOL3>(value));
      break;
    }
    case UniformType::BOOL4: {
      SetParameter(name, ParseFromString<UniformType::BOOL4>(value));
      break;
    }
  }
}

void MaterialInstance::InitDefaultInstance(Driver *driver, const Material *material) {
  driver_ = driver;

  material_ = material;
  if (!material->GetUniformBlock().IsEmpty()) {
    uniform_buffer_ = UniformBuffer(material->GetUniformBlock().GetSize());
    uniform_handle_ = driver->CreateBufferObject(uniform_buffer_.GetSize(),
                                                 BufferObjectBinding::UNIFORM,
                                                 BufferUsage::STATIC);
  }

  if (!material->GetSamplerBlock().IsEmpty()) {
    sampler_group_ = SamplerGroup(material->GetSamplerBlock().GetSize());
    sampler_handle_ = driver->CreateSamplerGroup(sampler_group_.GetSize());
  }

  const RasterState& raster_state = material->GetRasterState();
  culling_mode_ = raster_state.culling;
  color_write_ = raster_state.colorWrite;
  depth_write_ = raster_state.depthWrite;
  depth_func_ = raster_state.depthFunc;

  material_sorting_key_ = (((material_->GetId()) << 32) | material_->GenerateMaterialInstanceId());

  if (material->GetBlendingMode() == BlendingMode::MASKED) {
    SetMaskThreshold(material->GetAlphaMaskThreshold());
  }

  if (material->HasDoubleSidedCapability()) {
    SetDoubleSided(material->IsDoubleSidedMaterial());
  }

  // 给有默认值的参数赋值
  const auto& default_values = material_->GetDefaultParams();
  for (const auto& value : default_values) {
    SetDefaultValue(value.name, value.type, value.default_value);
    break;
  }
  SetParameter("baseColor", math::Vec4(0, 1, 0, 1));
}

void MaterialInstance::Destroy() {
  driver_->DestroyBufferObject(uniform_handle_);
  driver_->DestroySamplerGroup(sampler_handle_);
}

void MaterialInstance::Commit() const {
  if (uniform_buffer_.IsDirty()) {
    driver_->UpdateBufferObject(uniform_handle_, uniform_buffer_.ToBufferDescriptor(driver_), 0);
    uniform_buffer_.CleanFlag();
  }
  if (sampler_group_.IsDirty()) {
    driver_->UpdateSamplerGroup(sampler_handle_, std::move(sampler_group_.CopyAndClean()));
  }
}

void MaterialInstance::Use() const {
  if (uniform_handle_) {
    driver_->BindUniformBuffer(BindingPoints::PER_MATERIAL_INSTANCE, uniform_handle_);
  }

  if (sampler_handle_) {
    driver_->BindSamplers(BindingPoints::PER_MATERIAL_INSTANCE, sampler_handle_);
  }
}


}