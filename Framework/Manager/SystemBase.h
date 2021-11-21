//
// Created by Glodxy on 2021/11/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMBASE_H_
#include <map>
#include <string>
#include <vector>
#include "SystemEnum.h"
namespace our_graph {
class ComponentBase;

/**
 * 该类为system的功能性基类，
 * 定义了system的基础功能
 * */
class ISystem {
  using ComponentList = std::vector<std::shared_ptr<ComponentBase>>;
 public:
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
  // 所有需要关心的component
  // 在entity添加component时自动添加进system
  std::map<uint32_t, ComponentList> components_;

  constexpr static const char* SYSTEM_CALLER = "System";
  constexpr static int SYSTEM_CALLER_ID = 0;
};

/**
 * 该类为system的用于派生的基类，禁用所有的拷贝函数。
 * 该模板类用于限制系统的类型，限制所有的新system必须在已有的systemid中
 * */
template<SystemID ID>
class SystemBase : public ISystem {

 public:
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
  SystemID id_ = ID;
};




}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMBASE_H_
