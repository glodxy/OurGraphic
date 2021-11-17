//
// Created by Glodxy on 2021/11/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMBASE_H_

#include <string>
#include "SystemEnum.h"
namespace our_graph {
/**
 * 该类为ecs中s部分的基类，禁用所有的拷贝函数。
 * */
template<SystemID ID>
class SystemBase {
 public:
  virtual void Init() = 0;
  virtual void Destroy() = 0;

  SystemBase(const SystemBase& r) = delete;
  SystemBase(SystemBase&&) = delete;

  SystemBase& operator=(const SystemBase&) = delete;
  SystemBase& operator=(SystemBase&&) = delete;

  // 获取该系统的名称
  virtual std::string GetSystemName() const = 0;

  // 获取系统对应的id
  SystemID GetSystemID() const {
    return id_;
  }
 protected:
  SystemID id_ = ID;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMBASE_H_
