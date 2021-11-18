//
// Created by Glodxy on 2021/11/18.
//

#include "EntityManager.h"
#include "Entity.h"

namespace our_graph {

void EntityManager::Init() {}
void EntityManager::Destroy() {}

Entity EntityManager::AllocEntity() {
  uint32_t id = 0;
  {
    std::lock_guard<Mutex> lock_id(lock_id_);
    id = current_id_ ++;
  }
  std::set<std::shared_ptr<ComponentBase>>* list_ptr = nullptr;
  {
    std::lock_guard<Mutex> lock_map(lock_component_map_);
    entity_map_.try_emplace(id,std::set<std::shared_ptr<ComponentBase>>());
    list_ptr = &entity_map_[id];
  }
  return Entity(id, list_ptr);
}

void EntityManager::RemoveComponent(uint32_t id, ComponentBase *component) {
  std::lock_guard<Mutex> lock_map(lock_component_map_);
  auto iter = std::find_if(entity_map_[id].begin(), entity_map_[id].end(), [=](std::shared_ptr<ComponentBase> ptr)->bool {
    return ptr.get() == component;
  });
  if (iter != entity_map_[id].end()) {
    entity_map_[id].erase(iter);
  }
}

void EntityManager::RemoveAllComponent(uint32_t id) {
  std::lock_guard<Mutex> lock_map(lock_component_map_);
  entity_map_[id].clear();
}

void EntityManager::RemoveEntity(uint32_t id) {
  // 1. 清除该entity的所有component
  RemoveAllComponent(id);

  // 2. 移除该id
  std::lock_guard<Mutex> lock_map(lock_component_map_);
  entity_map_.erase(id);

  // 3. 添加到list
  removed_entities_.insert(id);
}

void EntityManager::GC() {
  // 1. 检查removed_entity，执行DestroyEntity
  for (const auto& entity : removed_entities_) {
    DestroyEntity(entity);
  }

  // 2. 清空removed_entity
  removed_entities_.clear();

  // 3. 析构removed_component
  for (auto com_ptr : removed_components_) {
    DestroyComponent(com_ptr);
  }

  // 4. 清空component
  removed_components_.clear();
}

void EntityManager::DestroyEntity(uint32_t id) {}
void EntityManager::DestroyComponent(ComponentBase *ptr) {
  delete ptr;
}

void EntityManager::Use(uint32_t id) {
  entity_use_cnt_[id] ++;
}

void EntityManager::UnUse(uint32_t id) {
  entity_use_cnt_[id] --;
}

EntityManager::EntityManager() {
  current_id_ = 0;
}

}  // namespace our_graph