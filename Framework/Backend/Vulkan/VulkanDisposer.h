//
// Created by Glodxy on 2021/10/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDISPOSER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDISPOSER_H_
#include <functional>
#include <map>
namespace our_graph {
/**
 * 该类用于资源的管理以及自动释放
 * */
class VulkanDisposer {
 public:
  using Key = const void*;
  using DestructFunc = std::function<void()>;

  /**
   * 将资源加入管理并设置引用数为1
   * */
  void CreateDisposable(Key resource, DestructFunc func) noexcept;

  void RemoveReference(Key resource) noexcept;

  /**
   * 请求资源，
   * 标识当前cmd buffer持有该资源，以便于之后的自动释放
   * */
  void Acquire(Key resource) noexcept;

  void GC() noexcept;

  void Reset() noexcept;

 private:
  struct Disposable {
    uint16_t ref_count = 1;
    uint16_t remain_frames = 0; //剩余帧数
    DestructFunc destructor = [](){};
  };

  std::map<Key, Disposable> disposables_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANDISPOSER_H_
