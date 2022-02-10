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

SkySource::SkySource(uint32_t id, const std::string &sky_file) : ComponentBase(id) {
  Driver* driver = IRenderProcessor::GetDriver();
  Texture* sky = TextureLoader::LoadCubeMap(driver, sky_file);
  skybox_ = Skybox::Builder(driver).Environment(sky).Build();
}

void SkySource::Init() {
  APICaller<SkySource>::RegisterAPIHandler(CALL_COMPONENT, entity_id_, weak_from_this());
}

Skybox *SkySource::GetSkybox() {
  return skybox_;
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