//
// Created by Glodxy on 2021/11/18.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_ENTITYMANAGER_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_ENTITYMANAGER_H_
#include <map>
#include <vector>
#include <set>
#include "Component/ComponentBase.h"
#include "Utils/Mutex.h"
namespace our_graph {
class Entity;
/**
 * 该类用于分配entity实例，
 * 并存储entity的实际数据，
 * entity的操作实际都相当于转发至该类
 * */
class EntityManager {
  using Mutex = utils::Mutex;
 public:
  static EntityManager& GetInstance();
  void Init();
  void Destroy();
 public:
  /**
   * 分配一个entity实例，
   * 该实例的id会存储至该类
   * */
  Entity AllocEntity();

  template<class T, typename = std::enable_if_t<std::is_base_of<T, ComponentBase>::value>,
      typename ...ARGS>
  T* AddComponent(uint32_t id, ARGS... args) {
    std::unique_lock<Mutex> lock(lock_component_map_);
    T* component = new T(id, std::forward<ARGS>(args)...);
    entity_map_[id].insert(component);
    return component;
  }

  /**
   * 目前仅将指针添加到remove list
   * 并将指针从set中移除
   * 实际的析构在每帧结束调用
   * */
  void RemoveComponent(uint32_t id, ComponentBase* component);
  void RemoveAllComponent(uint32_t id);

  /**
   * 每帧结束时调用对垃圾回收
   * 目前仅执行remove component
   * */
  void GC();

  /**
   * 增加引用计数
   * */
  void Use(uint32_t id);

  /**
   * 减少引用计数
   * */
  void UnUse(uint32_t id);

  /**
   * 仅将entity添加到removed list
   * 等待每帧结束时一同清理
   * */
   void RemoveEntity(uint32_t id);

 private:
  EntityManager();
  EntityManager(const EntityManager&) = delete;
  EntityManager(EntityManager&&) = delete;
  EntityManager& operator=(const EntityManager&) = delete;
  EntityManager& operator=(EntityManager&&) = delete;
  /**
   * 该函数仅在该类内部调用，
   * 目前在引用计数清0时进行调用
   * 销毁该entity所使用的资源
   * */
  void DestroyEntity(uint32_t id);

  /**
   * Component的实际销毁
   * */
   void DestroyComponent(ComponentBase* ptr);

  uint32_t current_id_ = 0; // 当前分配到的id值

  // 存储了所有entity对应的component
  std::map<uint32_t, std::set<ComponentBase*>> entity_map_;

  /**
   * entity的引用计数
   * 当计数清0时，代表该entity的所有实例都已销毁，则将其移除
   * 计数会在Entity类拷贝时增加
   * 在Entity析构时减少
   * */
   std::map<uint32_t, uint32_t> entity_use_cnt_;

   // 待删除的entity
   std::set<uint32_t> removed_entities_;
   // 所有待析构的component
   std::set<ComponentBase*> removed_components_;


   // todo: 减少锁的使用
   Mutex lock_component_map_;
   Mutex lock_id_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_ENTITYMANAGER_H_
