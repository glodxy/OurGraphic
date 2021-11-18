//
// Created by Glodxy on 2021/11/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_COMPONENTBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_COMPONENTBASE_H_
#include <string>
#include "Manager/SystemEnum.h"
namespace our_graph {
/**
 * 该类是ecs中c部分的基类
 * */
class ComponentBase {
 public:
  explicit ComponentBase(uint32_t id) : entity_id_(id) {}
  // 获取该component对应的system id
  virtual SystemID GetSystemID() = 0;
  virtual ~ComponentBase() = default;

  uint32_t GetEntity() const {
    return entity_id_;
  }
 protected:
  friend class EntityManager;
  // 注册函数
  virtual void Init() = 0;
  static constexpr const char* CALLER_TYPE = "Component";
  // entity的id
  uint32_t entity_id_;
};

}
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_COMPONENTBASE_H_
