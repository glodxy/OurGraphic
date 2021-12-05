//
// Created by Glodxy on 2021/12/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_MATERIAL_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_MATERIAL_H_
#include <string>
#include <vector>
#include "Backend/include/Driver.h"
#include "Backend/include/DriverEnum.h"
#include "Resource/base/BuilderBase.h"
#include "Resource/include_internal/ResourceBase.h"
#include "Resource/include_internal/MaterialEnum.h"
#include "Utils/Math/Math.h"
#include "Material/SamplerBlock.h"
#include "Material/UniformBlock.h"
namespace our_graph {
class MaterialInstance;
class Texture;
class TextureSampler;
class MaterialParser;
class Material : public ResourceBase {
  friend class ResourceAllocator;
  struct Details;
 public:
  using BlendingMode = BlendingMode;
  using Shading = Shading;

  using ParameterType = UniformType;
  using SamplerType = SamplerType;
  using SamplerFormat = SamplerFormat;
  using CullingMode = CullingMode;
  using SubpassType = SubpassType;

  /**
   * 每一个参数的描述
   * */
  struct ParameterInfo {
    std::string name;
    bool is_sampler; // 是否是采样器
    bool is_subpass; // 是否会用于subpass
    union {
      ParameterType type;
      SamplerType sampler_type;
      SubpassType subpass_type;
    };
    uint32_t count; // 该参数为array时的数量，默认为1
  };

  class Builder : public BuilderBase<Details> {
    friend class Material;
   public:
    Builder(Driver* driver) noexcept;
    Builder(const Builder& rhs) noexcept;
    Builder(Builder&& rhs) noexcept;
    ~Builder() noexcept;
    Builder& operator=(const Builder& rhs) noexcept;
    Builder& operator=(Builder&& rhs) noexcept;

    /**
     * 设置material的源数据
     * */
    Builder& Data(const void* data, size_t size);

    Material* Build();
  };

  ~Material() override;
  void Destroy() override;

 public:
  /**
   * 创建一个名称为name的material实例
   * 默认为空
   * 通常需要共享的instance需带有name
   * */
  MaterialInstance* CreateInstance(const std::string& name = "");

  // 获取该材质的名称
  std::string GetName() const noexcept {
    return name_;
  }
  // 获取着色模型
  Shading GetShading() const noexcept {
    return shading_;
  }
  // 获取混合模式
  BlendingMode GetBlendingMode() const noexcept {
    return blending_mode_;
  }
  // todo:render blending mode
  BlendingMode GetRenderBlendingMode() const noexcept {
    return render_blending_mode_;
  }
  // 获取材质的作用域
  MaterialDomain GetMaterialDomain() const noexcept {
    return material_domain_;
  }
  // 获取该材质的裁剪模式
  CullingMode GetCullingMode() const noexcept {
    return culling_mode_;
  }
  // 如何处理透明物体
  TransparencyMode GetTransparentMode() const noexcept {
    return transparency_mode_;
  }

  // 该材质是否会写入颜色缓冲
  bool IsColorWriteEnabled() const noexcept {
    return raster_state_.colorWrite;
  }
  // 是否写入深度缓冲
  bool IsDepthWriteEnable() const noexcept {
    return raster_state_.depthWrite;
  }
  // 是否启用深度裁剪
  bool IsDepthCullingEnabled() const noexcept {
    // 不是全通过即使用了裁剪
    return raster_state_.depthFunc != RasterState::DepthFunc::A;
  }
  // 是否是双面材质
  bool IsDoubleSidedMaterial() const noexcept {
    return double_sided_;
  }
  // 是否启用双面能力
  bool HasDoubleSidedCapability() const noexcept {
    return has_custom_depth_shader_;
  }
  // 获取alpha遮蔽的阈值
  float GetAlphaMaskThreshold() const noexcept {
    return mask_threshold_;
  }
  // 获取该材质必须的attribute槽
  AttributeBitset GetRequiredAttributes() const noexcept {
    return required_attributes_;
  }
  // 获取折射的模式
  RefractionMode GetRefractionMode() const noexcept {
    return refraction_mode_;
  }
  // 获取折射的类型
  RefractionType GetRefractionType() const noexcept {
    return refraction_type_;
  }

  // todo:property

  // 获取参数的数量
  size_t GetParameterCount() const noexcept {
    // todo:加上subpass
    return uniform_block_.GetUniformInfoList().size() +
          sampler_block_.GetSamplerInfoList().size();
  }
  /**
   * 获取该材质的参数信息
   * 以引用的形式返回获取到的参数信息
   * */
  void GetParameters(std::vector<ParameterInfo>& parameters);

  // 判断是否拥有某参数
  bool HasParameter(const std::string& name) const noexcept;
  // 判断某参数是否是sampler
  bool IsSampler(const std::string& name) const noexcept;

  /**
   * 设置普通参数
   * @param name：参数名
   * @param value：参数值
   * */
  template<class T>
  void SetDefaultParameter(const std::string& name, T value) noexcept {
    GetDefaultInstance()->SetParameter(name, value);
  }
  /**
   * 设置texture参数，需要配套设置sampler
   * */
  void SetDefaultParameter(const std::string& name, Texture const* texture,
                           TextureSampler const& sampler) noexcept {
    GetDefaultInstance()->SetParameter(name, texture, sampler);
  }

  /**
   * 设置颜色参数, float3
   * */
  void SetDefaultParameter(const std::string& name, math::Vec3 color) noexcept {
    GetDefaultInstance()->SetParameter(name, color);
  }

  void SetDefaultParameter(const std::string& name, math::Vec4 color) noexcept {
    GetDefaultInstance()->SetParameter(name, color);
  }

  MaterialInstance* GetDefaultInstance() noexcept {
    return &default_instance_;
  }
  MaterialInstance const * GetDefaultInstance() const noexcept {
    return &default_instance_;
  }

  ShaderHandle GetShader(uint8_t key) const noexcept {
    ShaderHandle handle = cache_programs_[key];
    return handle ? handle : BuildShader(key);
  }

  // 生成instance的id
  uint32_t GenerateMaterialInstanceId() const noexcept {
    return material_instance_id_++;
  }

  /**
   * 应用所有未提交的修改
   * */
  void ApplyPendingEdits();
 private:
  Material(const Builder& builder);
  /**
   * 根据cache中的内容构建shader并提交至gpu
   * */
  ShaderHandle BuildShader(uint8_t key) const noexcept;
  // 缓存的着色器程序，按照使用频率排序
  mutable std::array<ShaderHandle, 128> cache_programs_;

  RasterState raster_state_;
  BlendingMode render_blending_mode_ = BlendingMode::OPAQUE;
  TransparencyMode transparency_mode_ = TransparencyMode::DEFAULT;
  Shading shading_ = Shading::UNLIT;

  // todo
  BlendingMode blending_mode_ = BlendingMode::OPAQUE;
  MaterialDomain material_domain_ = MaterialDomain::SURFACE;
  CullingMode culling_mode_ = CullingMode::NONE;
  AttributeBitset required_attributes_;
  RefractionMode refraction_mode_ = RefractionMode::NONE;
  RefractionType refraction_type_ = RefractionType::SOLID;
  // todo
  uint64_t material_properties_ = 0;

  float mask_threshold_ = 0.4f;

  // todo
  bool double_sided_ = false;
  bool double_sided_capability_ = false;
  bool has_custom_depth_shader_ = false;
  bool is_default_material = false;

  MaterialInstance default_instance_;
  // todo: sampler&uniform block
  UniformBlock uniform_block_;
  SamplerBlock sampler_block_;
  // todo:subpass info
  // todo:sampler binding map

  std::string name_;
  Driver* driver_ {nullptr};
  const uint32_t material_id_;
  mutable uint32_t material_instance_id_ = 0;

  // todo:material parser
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_MATERIAL_H_
