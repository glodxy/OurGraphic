//
// Created by Glodxy on 2021/9/7.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ICOMMANDPOOL_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ICOMMANDPOOL_H_
#include <vector>
#include "ICommand.h"
#include "ICommandBuffer.h"

namespace our_graph {
/**
 * 该类用于管理CommandBuffer，用于自动选择可用Buffer，
 * 自动提交command至GPU。
 * 提供给外部的接口为：AddCommand。
 * 该类会管理多个由ICommandBuffer与实际的CommandBuffer构成的集合
 * */
class ICommandPool {
 public:
  virtual void Create() = 0;
  virtual void Destroy() = 0;
  virtual void AddCommand(std::shared_ptr<ICommand> command) = 0;
  virtual ICommandBuffer* GetBuffer() = 0;
 protected:

};
}
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ICOMMANDPOOL_H_
