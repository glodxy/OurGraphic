//
// Created by Glodxy on 2021/11/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_COMPONENTBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_COMPONENTBASE_H_
#include <memory>
#include <string>
#include <type_traits>
#include "Manager/SystemEnum.h"
namespace our_graph {
static constexpr const char* CALL_COMPONENT = "Component";
/**
 * 组件的类型
 * 只有几种固定的类型需要标记，
 * 其他可以用Unknown或自定义标记
 * */
enum ComponentType {
  UNKNOWN = 0,
  CAMERA = 1,
  TRANSFORM = 2,
  RENDERABLE = 3,
  LIGHT = 4,
  SKY = 5,
  MAX = 50, // 内置的最大类型数
};

/**
 * 该类是ecs中c部分的基类
 * 目前ecs设计仅支持单个Component的实例
 * 即对于每种component， system只支持处理entity的一个实例
 * */
class ComponentBase {
 public:
  explicit ComponentBase(uint32_t id) : entity_id_(id) {}
  // 获取该component对应的system id
  virtual SystemID GetSystemID() const = 0;
  virtual ~ComponentBase() = default;

  uint32_t GetEntity() const {
    return entity_id_;
  }

  virtual uint32_t GetComponentType() const {
    return UNKNOWN;
  }

  /**
   * 该函数为初始化函数，通常用于向APICaller注册
   * 以方便同一entity的component之间进行调用。
   * */
  virtual void Init() = 0;
 protected:
  friend class EntityManager;
  // entity的id
  uint32_t entity_id_;
};

template<class T, typename = std::enable_if_t<std::is_base_of_v<ComponentBase, T>>>
std::shared_ptr<T> ComCast(std::shared_ptr<ComponentBase> com) {
  return std::dynamic_pointer_cast<T>(com);
}


}
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_COMPONENTBASE_H_
