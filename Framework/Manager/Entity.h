//
// Created by Glodxy on 2021/11/18.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_ENTITY_H_
#define OUR_GRAPHIC_FRAMEWORK_ENTITY_H_
#include <vector>
#include <type_traits>
#include <bitset>

#include "EntityManager.h"
namespace our_graph {
class ComponentBase;
class Entity {
  static constexpr uint32_t NULL_ENTITY = std::numeric_limits<uint32_t>::max();
 public:
  class Builder {
   public:
    static Entity Build();
  };

 public:
  /**
   * 添加组件，组件必须继承自ComponentBase
   * */
  template<typename T, typename ...ARGS>
  std::shared_ptr<T> AddComponent(ARGS&&... args) {
    return EntityManager::GetInstance().AddComponent<T>(instance_id_, std::forward<ARGS>(args)...);
  }

  /**
   * 获取指定类型的组件
   * */
  template<class T, typename = std::enable_if_t<std::is_base_of<ComponentBase, T>::value>>
  std::shared_ptr<T> GetComponent() {
    for (auto iter : *components_) {
      if (typeid(*iter) == typeid(T)) {
        return ComCast<T>(iter);
      }
    }
    return nullptr;
  }

  bool IsNull() {
    return instance_id_ == NULL_ENTITY;
  }

  ~Entity();
  // 允许拷贝构造
  // 因为此处都要添加引用计数，故不区分左值右值
  Entity(const Entity& r);

  uint32_t GetInstanceID() const {return instance_id_;}
 protected:
  friend class EntityManager;
  Entity() = delete;
  explicit Entity(uint32_t id, std::set<std::shared_ptr<ComponentBase>>* ptr);
  // 该entity的实例id
  const uint32_t instance_id_;
  // 该list实际存储于EntityMgr，此处仅维护一个指针以减少传输的消耗
  // 尽可能缩小该类的size
  std::set<std::shared_ptr<ComponentBase>>* const components_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_ENTITY_H_
