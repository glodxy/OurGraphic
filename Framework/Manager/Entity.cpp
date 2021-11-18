//
// Created by Glodxy on 2021/11/18.
//

#include "Entity.h"

namespace our_graph {

Entity Entity::Builder::Build() {
  return EntityManager::GetInstance().AllocEntity();
}

Entity::Entity(uint32_t id, std::set<std::shared_ptr<ComponentBase>>*ptr) : instance_id_(id), components_(ptr) {
  EntityManager::GetInstance().Use(id);
}

Entity::Entity(const Entity &r) : instance_id_(r.instance_id_), components_(r.components_) {
  EntityManager::GetInstance().Use(r.instance_id_);
}

Entity::~Entity() {
  uint32_t id = instance_id_;
  EntityManager::GetInstance().UnUse(id);
}

}