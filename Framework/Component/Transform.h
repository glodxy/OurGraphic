//
// Created by Glodxy on 2021/11/18.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_TRANSFORM_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_TRANSFORM_H_
#include <memory>
#include "ComponentBase.h"
#include "Utils/Math/Math.h"
namespace our_graph {
class Transform : public ComponentBase,
 public std::enable_shared_from_this<Transform> {
 public:
  explicit Transform(uint32_t id);
  ~Transform() override;

  SystemID GetSystemID() const override {return SystemID::NONE;}
  uint32_t GetComponentType() const override;
  math::Vec3 GetPosition();

  void SetPosition(math::Vec3 pos);
 protected:
  void Init() override;
 private:

  math::Vec3 position_ {0, 0, 1.3f};
  math::Vec3 rotate_;
  math::Vec3 scale_;
};
}

#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_TRANSFORM_H_
