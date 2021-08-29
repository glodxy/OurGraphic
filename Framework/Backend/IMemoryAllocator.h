//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_IMEMORYALLOCATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_IMEMORYALLOCATOR_H_
#include <unordered_map>

#include "IRenderDevice.h"
#include "IMemoryHandle.h"
namespace our_graph {
/**
 * 该类为单例类,用于管理显存的分配与销毁
 * */
class IMemoryAllocator {
 public:
  virtual void Init(std::shared_ptr<IRenderDevice> device) = 0;
  virtual void Clear() = 0;
  virtual std::shared_ptr<IMemoryHandle> AllocateGPUMemory(const std::string& name, uint64_t size) = 0;
  virtual void DestroyGPUMemory(const std::string& name) = 0;

  /**
   *将显存与本地的内存映射
   * */
  virtual bool MapGPUMemoryToLocal(const std::string& name, void** p_local) = 0;
  virtual bool UnMapGPUMemory(const std::string& name) = 0;

  template<class T, typename =
  std::enable_if<std::is_base_of<IMemoryAllocator, T>::value>>
  static std::shared_ptr<T> Get() {
    static std::shared_ptr<T> allocator =
        std::make_shared<T>(token());
    return allocator;
  }

 protected:
  explicit IMemoryAllocator() = default;
  struct token{};
  std::unordered_map<std::string, std::shared_ptr<IMemoryHandle>> memory_map_;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_IMEMORYALLOCATOR_H_
