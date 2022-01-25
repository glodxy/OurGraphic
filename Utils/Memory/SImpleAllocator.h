//
// Created by Glodxy on 2022/1/20.
//

#ifndef OUR_GRAPHIC_UTILS_MEMORY_SIMPLEALLOCATOR_H_
#define OUR_GRAPHIC_UTILS_MEMORY_SIMPLEALLOCATOR_H_
#include <memory>
#include <vector>
namespace our_graph::utils {
class SimpleAllocator {
 public:
  template<class T, typename...ARGS>
  T* Make(ARGS&&... args) {
    void* const data = ::malloc(sizeof(T));
    return data ? new(data) T(std::forward<ARGS>(args)...) : nullptr;
  }

  template<class T>
  void Destroy(T* p) noexcept {
    if (p) {
      p->~T();
      ::free(p);
    }
  }

 private:
};
}
#endif //OUR_GRAPHIC_UTILS_MEMORY_SIMPLEALLOCATOR_H_
