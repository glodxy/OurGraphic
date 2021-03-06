//
// Created by Glodxy on 2021/11/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMMANAGER_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMMANAGER_H_
#include "SystemBase.h"
#include <map>
#include <memory>

#include "Backend/include/Driver.h"
namespace our_graph {
class SystemManager {
 public:
  static SystemManager& GetInstance() {
    static SystemManager mgr;
    return mgr;
  }
  /**
   * 初始化所有的system并实例化
   * 目前使用SystemEnum.h中的初始化list来初始化
   * */
  void Init(Driver* driver);
  // 销毁所有的system
  void Close();

  // 获取对应的system实例
  ISystem* GetSystem(SystemID id);

  /**
   * 每帧的调用
   * */
  void Update(uint32_t frame);

 private:
  SystemManager();
  Driver* driver_{nullptr};
  std::map<SystemID, std::shared_ptr<ISystem>> system_map_;
};

}
#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMMANAGER_H_
