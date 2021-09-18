//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_ICOMMANDBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_ICOMMANDBUFFER_H_
#include <memory>
#include <vector>
#include "ICommand.h"
namespace our_graph {
/**
 * 该类为虚拟CommandBuffer，
 * 只用于缓存提交的自定Command。
 * */
class ICommandBuffer {
 public:
  virtual ~ICommandBuffer() = default;
  virtual void AddCommand(std::shared_ptr<ICommand> command) = 0;
  // 判断是否可用，即是否可向其中添加command
  virtual bool IsAvailable() const = 0;
  // 设置buffer当前状态
  virtual void SetState(int state) = 0;
  // 是否已满
  virtual bool IsFull() const = 0;
  // 清空
  virtual void Clear() = 0;
  // 获取list
  virtual std::vector<std::shared_ptr<ICommand> > GetList() const = 0;

  // 开始使用
  virtual void Use() = 0;
  // 停止使用
  virtual void EndUse() = 0;
  // 获取实例指针
  virtual void* GetInstance() = 0;
 protected:

};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_ICOMMANDBUFFER_H_
