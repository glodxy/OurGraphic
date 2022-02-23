//
// Created by Glodxy on 2022/2/9.
//

#include "SkySource.h"
#include "Resource/include/Texture.h"
#include "Resource/include/Skybox.h"
#include "Resource/include/TextureLoader.h"
#include "IRenderProcessor.h"
#include "Utils/Event/APICaller.h"

namespace our_graph {
using utils::APICaller;

SkySource::SkySource(uint32_t id, const std::string &sky_file, IBLFile* ibl) : ComponentBase(id) {
  Driver* driver = IRenderProcessor::GetDriver();
  Texture* sky = TextureLoader::LoadCubeMap(driver, sky_file);
  skybox_ = Skybox::Builder(driver).Environment(sky).Build();

  if (ibl != nullptr) {
    ibl_tex_.diffuse = TextureLoader::LoadCubeMap(driver, ibl->diffuse_file);
    ibl_tex_.specular = TextureLoader::LoadCubeMap(driver, ibl->specular_file, 5);
    ibl_tex_.brdf = TextureLoader::LoadTexture(driver, ibl->brdf_lut);
  }
}

void SkySource::Init() {
  APICaller<SkySource>::RegisterAPIHandler(CALL_COMPONENT, entity_id_, weak_from_this());
}

Skybox *SkySource::GetSkybox() {
  return skybox_;
}

IBLTex SkySource::GetIBL() {
  return ibl_tex_;
}

SkySource::~SkySource() noexcept {
  APICaller<SkySource>::RemoveAPIHandler(CALL_COMPONENT, entity_id_);
}

uint32_t SkySource::GetComponentType() const {
  return ComponentType::SKY;
}

SystemID SkySource::GetSystemID() const {
  return SystemID::RENDER;
}

}