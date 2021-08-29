//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_IMEMORYHANDLE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_IMEMORYHANDLE_H_
#include <string>
namespace our_graph {
/**
 * 显存句柄，通过MemoryAllocator返回
 * */
class IMemoryHandle {
 public:

  std::string GetName() const {
    return memory_name_;
  }

  uint64_t GetSize() const {
    return memory_size_;
  }

  virtual ~IMemoryHandle() = default;
 protected:
  std::string memory_name_;
  uint64_t memory_size_;
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_IMEMORYHANDLE_H_
