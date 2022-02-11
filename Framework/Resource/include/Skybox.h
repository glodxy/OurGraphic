//
// Created by Glodxy on 2022/2/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_SKYBOX_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_SKYBOX_H_
#include "Resource/include_internal/ResourceBase.h"
#include "Resource/base/BuilderBase.h"
#include "Backend/include/Driver.h"
#include "Utils/Math/Math.h"
namespace our_graph {
class Texture;
class MaterialInstance;
class Skybox : public ResourceBase {
  friend class ResourceAllocator;
  struct Detail;

 public:
  class Builder : public BuilderBase<Detail> {
    friend class Detail;
    friend class Skybox;

   public:
    Builder(Driver* driver) noexcept;
    Builder(Builder const& rhs) noexcept;
    Builder(Builder&& rhs) noexcept;
    ~Builder() noexcept;
    Builder& operator=(Builder const& rhs) noexcept;
    Builder& operator=(Builder&& rhs) noexcept;

    /**
     * 设置环境的cubemap纹理
     * */
     Builder& Environment(Texture* tex);

     /**
      * 是否显示太阳
      * */
     Builder& HasSun(bool f);

     /**
      * 没有间接光照时的光强
      * */
     Builder& Intensity(float intensity);

     /**
      * 没有环境贴图时的默认颜色
      * */
     Builder& Color(math::Vec4 color);

     Skybox* Build();
  };

  void Destroy() override;

  // 获取光强
  float GetIntensity() const;
  // 获取对应的tex
  const Texture* GetTexture() const;

  MaterialInstance* GetMaterialInstance() const;

 private:
  explicit Skybox(const Builder& builder);
  Texture* env_;
  float intensity_;
  math::Vec4 color_;
  bool has_sun_;

  // 天空盒的材质
  MaterialInstance* mat_;
  Driver* driver_ = nullptr;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_SKYBOX_H_
