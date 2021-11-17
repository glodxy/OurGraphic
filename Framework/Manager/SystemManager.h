//
// Created by Glodxy on 2021/11/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMMANAGER_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMMANAGER_H_
#include "SystemBase.h"
#include <map>
namespace our_graph {
class SystemManager {
 public:
  static SystemManager& GetInstance();
  /**
   * 初始化所有的system并实例化
   * 目前使用SystemEnum.h中的初始化list来初始化
   * */
  void Init();
  // 销毁所有的system
  void Close();

  // 获取对应的system实例
  template<SystemID id>
  SystemBase<id>* GetSystem();
 private:
  SystemManager();

  std::map<SystemID, void*> system_map_;
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMMANAGER_H_
