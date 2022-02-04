//
// Created by Glodxy on 2022/2/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_LIGHTSOURCE_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_LIGHTSOURCE_H_
#include "ComponentBase.h"
#include "Utils/Math/Math.h"
namespace our_graph {
class LightSource : public ComponentBase ,
 public std::enable_shared_from_this<LightSource> {
 public:
  explicit LightSource(uint32_t id);
  ~LightSource() override;

  void Init() override;

  uint32_t GetComponentType() const override;
  SystemID GetSystemID() const override;

  // 获取该light对应的mat表示
  math::Mat4 GetLightMat() const;

  bool IsStatic() const;
 private:
  bool is_static_ = false;  // 是否是静态光源

  float falloff; // 1/falloff
  math::Vec4 color {1, 0, 1, 1};
  math::Vec3 direction;
  float intensity = 10;
  math::Vec4 padding;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_LIGHTSOURCE_H_
