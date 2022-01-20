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
    void* data = ::malloc(sizeof(T));
    new(data) T(std::forward<ARGS>(args)...);

    return data;
  }

  template<class T>
  void Destroy(T* p) noexcept {
    if (p) {
      p->~T();
      ::free(p, sizeof(T));
    }
  }

 private:
};
}
#endif //OUR_GRAPHIC_UTILS_MEMORY_SIMPLEALLOCATOR_H_
