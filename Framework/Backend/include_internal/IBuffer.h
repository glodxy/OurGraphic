//
// Created by Glodxy on 2021/8/31.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_IBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_IBUFFER_H_
#include <memory>
#include "Framework/Backend/include/MemoryHandle.h"

namespace our_graph {
/**
 * Buffer用于描述如何去从Memory中取出相应的数据，以及指定相应的绑定方式
 * 通常与Descriptor一同构成Resource
 * */
class IBuffer {
 public:
  virtual void Create() {

  }

  virtual ~IBuffer(){};
  void SetMemory(std::shared_ptr<MemoryHandle> memory) {
    memory_ = memory;
  }

  virtual std::shared_ptr<MemoryHandle> GetMemory() {
    return memory_;
  }

  virtual void* GetInstance() {
    return nullptr;
  }
 protected:
  std::shared_ptr<MemoryHandle> memory_ {nullptr};
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_IBUFFER_H_
