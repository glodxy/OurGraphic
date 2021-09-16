//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_IMEMORYALLOCATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_IMEMORYALLOCATOR_H_
#include <unordered_map>

#include "IRenderDevice.h"
#include "MemoryHandle.h"
namespace our_graph {
/**
 * 该类为单例类,用于管理显存的分配与销毁
 * */
class MemoryAllocator {
 public:
  virtual void Init(std::shared_ptr<IRenderDevice> device) = 0;
  virtual void Clear() = 0;
  /**
   * 手动或自动选择显存并分配
   * */
  virtual std::shared_ptr<MemoryHandle> AllocateGPUMemoryByIdx(const std::string& name,
                                                          uint64_t size,
                                                          int memory_idx = -1) = 0;
  /**
   * 按照显存类型选择显存并分配
   * */
  virtual std::shared_ptr<MemoryHandle> AllocateGPUMemoryByType(const std::string& name,
                                                          uint64_t size,
                                                          uint64_t type_flag) = 0;
  virtual void DestroyGPUMemory(const std::string& name) = 0;

  /**
   *将显存与本地的内存映射
   * */
  virtual bool MapGPUMemoryToLocal(const std::string& name, void** p_local) = 0;
  virtual bool UnMapGPUMemory(const std::string& name) = 0;

  template<class T, typename =
  std::enable_if<std::is_base_of<MemoryAllocator, T>::value>>
  static std::shared_ptr<T> Get() {
    static std::shared_ptr<T> allocator =
        std::make_shared<T>(token());
    return allocator;
  }

  template<class T, typename =
  std::enable_if<std::is_base_of<MemoryHandle, T>::value>>
  std::shared_ptr<T> GetMemory(const std::string& name) {
    std::shared_ptr<T> res = nullptr;
    std::shared_ptr<MemoryHandle> tmp = memory_map_[name];
    if (!tmp) {
      return res;
    }
    res = std::dynamic_pointer_cast<T>(tmp);
    return res;
  }
 protected:
  explicit MemoryAllocator() = default;
  struct token{};
  std::unordered_map<std::string, std::shared_ptr<MemoryHandle>> memory_map_;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_IMEMORYALLOCATOR_H_
