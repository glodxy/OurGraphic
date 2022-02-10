//
// Created by Glodxy on 2022/2/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_SKYSOURCE_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_SKYSOURCE_H_
#include "ComponentBase.h"
#include <memory>
namespace our_graph {
class Skybox;
class SkySource : public ComponentBase ,
 public std::enable_shared_from_this<SkySource> {
 public:
  SkySource(uint32_t id, const std::string& sky_file);
  Skybox* GetSkybox();

  void Init() override;
 public:
  ~SkySource() override;
  uint32_t GetComponentType() const override;
  SystemID GetSystemID() const override;

 private:
  Skybox* skybox_ = nullptr;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_SKYSOURCE_H_
