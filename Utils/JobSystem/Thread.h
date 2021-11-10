//
// Created by Glodxy on 2021/10/19.
//

#ifndef OUR_GRAPHIC_UTILS_JOBSYSTEM_THREAD_H_
#define OUR_GRAPHIC_UTILS_JOBSYSTEM_THREAD_H_
#include <thread>

namespace our_graph::utils {
class Thread {
 public:
  Thread();
  virtual ~Thread();

  bool Start();
  virtual void Run() = 0;

 protected:
  std::thread* thread_;
};
}
#endif //OUR_GRAPHIC_UTILS_JOBSYSTEM_THREAD_H_
