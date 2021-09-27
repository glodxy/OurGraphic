//
// Created by Glodxy on 2021/9/26.
//

#ifndef OUR_GRAPHIC_UTILS_MUTEX_H_
#define OUR_GRAPHIC_UTILS_MUTEX_H_
#include <condition_variable>
#include <mutex>
namespace our_graph {
namespace utils {
class Condition : public std::condition_variable {
 public:
  using std::condition_variable::condition_variable;

  inline void notify_n(size_t n) noexcept {
    if (n == 1) {
      notify_one();
    } else if (n > 1) {
      notify_all();
    }
  }
};

using Mutex = std::mutex;
}
}

#endif //OUR_GRAPHIC_UTILS_MUTEX_H_
