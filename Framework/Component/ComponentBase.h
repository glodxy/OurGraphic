//
// Created by Glodxy on 2021/11/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_COMPONENTBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_COMPONENTBASE_H_
#include "Manager/SystemEnum.h"
namespace our_graph {
/**
 * 该类是ecs中c部分的基类
 * */
class ComponentBase {
 public:
  // 获取该component对应的system id
  virtual SystemID GetSystemID() = 0;
 protected:

};

}
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_COMPONENTBASE_H_
