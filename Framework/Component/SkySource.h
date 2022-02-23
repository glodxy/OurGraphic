//
// Created by Glodxy on 2022/2/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_SKYSOURCE_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_SKYSOURCE_H_
#include "ComponentBase.h"
#include <memory>
namespace our_graph {
class Skybox;
class Texture;

struct IBLTex {
  Texture* diffuse = nullptr;
  Texture* specular = nullptr;
  Texture* brdf = nullptr;
};
/**
 * 该组件全局唯一
 * */
class SkySource : public ComponentBase ,
 public std::enable_shared_from_this<SkySource> {
 public:
  struct IBLFile {
    std::string diffuse_file;  // diffuse irradiance
    std::string specular_file;  // specular prefilter
    std::string brdf_lut;
  };

  SkySource(uint32_t id, const std::string& sky_file, IBLFile* ibl = nullptr);
  Skybox* GetSkybox();

  IBLTex GetIBL();

  void Init() override;
 public:
  ~SkySource() override;
  uint32_t GetComponentType() const override;
  SystemID GetSystemID() const override;

 private:
  Skybox* skybox_ = nullptr;
  IBLTex ibl_tex_;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_SKYSOURCE_H_
