//
// Created by Glodxy on 2021/12/7.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_MATERIALINSTANCE_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_MATERIALINSTANCE_H_
#include <string>
#include "Framework/Resource/include_internal/ResourceBase.h"
#include "Framework/Backend/include/DriverEnum.h"
#include "Framework/Backend/include/Driver.h"

#include "Framework/Resource/include/Texture.h"
#include "Framework/Resource/include/TextureSampler.h"
#include "Framework/Resource/Material/UniformBuffer.h"
#include "Utils/Math/Math.h"
namespace our_graph {
class Material;
class MaterialInstance : public ResourceBase {
  friend class Material;
  friend class ResourceAllocator;
  using CullingMode = CullingMode;

  // 判断是否是支持的类型
  template<typename T>
  using is_supported_parameter_t = typename std::enable_if<
      std::is_same<float, T>::value ||
          std::is_same<int32_t, T>::value ||
          std::is_same<uint32_t, T>::value ||
          std::is_same<math::Vec2i, T>::value ||
          std::is_same<math::Vec3i, T>::value ||
          std::is_same<math::Vec4i, T>::value ||
          std::is_same<math::Vec2u, T>::value ||
          std::is_same<math::Vec3u, T>::value ||
          std::is_same<math::Vec4u, T>::value ||
          std::is_same<math::Vec2, T>::value ||
          std::is_same<math::Vec3, T>::value ||
          std::is_same<math::Vec4, T>::value ||
          std::is_same<math::Mat4 , T>::value ||
          // these types are slower as they need a layout conversion
          std::is_same<bool, T>::value ||
          std::is_same<math::Vec2b, T>::value ||
          std::is_same<math::Vec3b, T>::value ||
          std::is_same<math::Vec4b, T>::value ||
          std::is_same<math::Mat3, T>::value
  >::type;
 public:
  MaterialInstance(const MaterialInstance&) = delete;
  MaterialInstance& operator=(const MaterialInstance&) = delete;

  /**
   * 从一个已有的instance拷贝出一个名字为name的副本
   * */
  static MaterialInstance* Duplicate(const MaterialInstance* src,
                                     const std::string& name) noexcept;

  void Destroy() override;
  ~MaterialInstance() override;

  /**
   * 向driver提交该material的变更
   * 只是将资源提交至driver，并未真正使用
   * */
  void Commit() const;
  /**
   * 向Driver提交，绑定该资源
   * 此处才真正的使用资源
   * */
  void Use() const;

 public:
  const Material* GetMaterial() const noexcept {return material_;}
  uint64_t GetSortingKey() const noexcept {return material_sorting_key_;}
  const UniformBuffer& GetUniformBuffer() const noexcept {return uniform_buffer_;}
  const SamplerGroup& GetSamplerGroup() const noexcept {return sampler_group_;}
  uint32_t GetId() const noexcept {return id_;}

  CullingMode GetCullingMode() const noexcept {return culling_mode_;}
  bool GetColorWrite() const noexcept {return color_write_;}
  bool GetDepthWrite() const noexcept {return depth_write_;}
  RasterState::DepthFunc GetDepthFunc() const noexcept {return depth_func_;}
  std::string GetName() const noexcept;

  void SetMaskThreshold(float threshold) noexcept;
  void SetDoubleSided(bool double_sided) noexcept;
  void SetColorWrite(bool color_write) noexcept {color_write_ = color_write;}
  void SetDepthWrite(bool depth_write) noexcept {depth_write_ = depth_write;}
  void SetCullingMode(CullingMode mode) noexcept {culling_mode_ = mode;}
  // todo
  void SetDepthCulling(bool enable) noexcept;
 public:
  void SetParameter(const std::string& name,
                    TextureHandle handle,
                    SamplerParams param);

  /**
   * 设置单个参数
   * */
  template<typename T, typename = is_supported_parameter_t<T>>
  void SetParameter(const std::string& name,
                    const T& value);

  /**
   * 设置array形式的参数
   * */
  template<typename T, typename = is_supported_parameter_t<T>>
  void SetParameter(const std::string& name,
                    const T* value,
                    size_t size);
  /**
   * 设置纹理参数
   * */
  void SetParameter(const std::string& name,
                    const Texture* texture,
                    const TextureSampler& sampler);

 protected:
  /**
   * 只允许从其他material instance复制
   * */
  MaterialInstance(const MaterialInstance* src,
                   const std::string& name);
  MaterialInstance() = default;
  void InitDefaultInstance(Driver* driver, const Material* material);
 private:
  template<size_t S>
  void SetParameterUntyped(const std::string& name,
                           const void* value) noexcept;

  template<size_t S>
  void SetParameterUntyped(const std::string& name,
                           const void* value,
                           size_t cnt) noexcept;




 private:
  Driver* driver_ {nullptr};
  const Material* material_{nullptr};
  BufferObjectHandle uniform_handle_;
  SamplerGroupHandle sampler_handle_;

  UniformBuffer uniform_buffer_;
  SamplerGroup sampler_group_;

  // 以下参数可以被instance覆盖
  CullingMode culling_mode_;
  bool color_write_ = true;
  bool depth_write_ = false;
  RasterState::DepthFunc depth_func_;

  /**
   * 该key用于material之间的排序
   * 会影响实际的渲染顺序
   * */
  uint64_t material_sorting_key_ = 0;
  // 用于标识唯一的material instance，在scene中查找用
  uint32_t id_ = 0;
  std::string name_;
};

}
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_MATERIALINSTANCE_H_
