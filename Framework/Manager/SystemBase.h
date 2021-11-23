//
// Created by Glodxy on 2021/11/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMBASE_H_
#include <map>
#include <string>
#include <vector>
#include <memory>
#include "SystemEnum.h"
#include "Component/ComponentBase.h"
#include "Backend/include/Driver.h"
namespace our_graph {
class ComponentBase;

/**
 * 该类为system的功能性基类，
 * 定义了system的基础功能
 * */
class ISystem {
  using ComponentList = std::vector<std::shared_ptr<ComponentBase>>;
 public:
  ISystem() = default;
  virtual void Init() = 0;
  virtual void Destroy() = 0;

  /**
   * 将entity的component指针添加进map
   * 方便之后的逻辑处理
   * 在实际的system逻辑中，只需关心这个map中的component
   * */
  void AddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) {
    OnAddComponent(id, com);
    components_[id].push_back(com);
  }

 protected:
  // 添加组件到system到回调
  // 可以在这里处理自定义事件
  virtual void OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) {}

  // 获取对应的component
  std::shared_ptr<ComponentBase> GetComponent(uint32_t id, ComponentType type) {
    auto find_func = [=](std::shared_ptr<ComponentBase> com) -> bool {
      if (com->GetComponentType() == type) {
        return true;
      }
      return false;
    };
    return *std::find_if(components_[id].begin(), components_[id].end(), std::move(find_func));
  }

  // 获取对应的component
  std::shared_ptr<ComponentBase> GetComponentFromList(const ComponentList& list, ComponentType type) {
    auto find_func = [=](std::shared_ptr<ComponentBase> com) -> bool {
      if (com->GetComponentType() == type) {
        return true;
      }
      return false;
    };
    return *std::find_if(list.begin(), list.end(), std::move(find_func));
  }

  // 所有需要关心的component
  // 在entity添加component时自动添加进system
  std::map<uint32_t, ComponentList> components_;

  constexpr static const char* SYSTEM_CALLER = "System";
  constexpr static int SYSTEM_CALLER_ID = 0;
  static Driver* driver_; // 所有system共享的driver，由SystemManager初始化时设置
};

/**
 * 该类为system的用于派生的基类，禁用所有的拷贝函数。
 * 该模板类用于限制系统的类型，限制所有的新system必须在已有的systemid中
 * */
template<SystemID ID>
class SystemBase : public ISystem {

 public:
  SystemBase() = default;
  SystemBase(const SystemBase& r) = delete;
  SystemBase(SystemBase&&) = delete;

  SystemBase& operator=(const SystemBase&) = delete;
  SystemBase& operator=(SystemBase&&) = delete;

  // 获取该系统的名称
  virtual std::string GetSystemName() const = 0;

  // 获取系统对应的id
  SystemID GetSystemID() const {
    return ID;
  }
 protected:
  virtual void Init() = 0;
  virtual void Destroy() = 0;
  virtual void OnAddComponent(uint32_t id, std::shared_ptr<ComponentBase> com) {}
  SystemID id_ = ID;
};




}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMBASE_H_
