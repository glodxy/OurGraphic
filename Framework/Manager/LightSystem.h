//
// Created by Glodxy on 2022/2/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_LIGHTSYSTEM_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_LIGHTSYSTEM_H_
#include <memory>
#include <vector>
#include "Component/LightSource.h"
#include "SystemBase.h"
namespace our_graph {
/**
 *该system用于管理全局的light
 * */
class LightSystem : public SystemBase<SystemID::LIGHT> ,
 public std::enable_shared_from_this<LightSystem> {
 public:
  void Init() override;
  void Destroy() override;
  std::string GetSystemName() const override {
    return "LightSystem";
  }

  // 获取所有的动态光源
  std::vector<std::shared_ptr<LightSource>> GetAllDynamicLight();
 protected:
  // 在此处将light source加入管理
  void OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) override;

 private:
  // 存储所有的动态光源
  std::vector<std::shared_ptr<LightSource>> dynamic_lights_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_LIGHTSYSTEM_H_
