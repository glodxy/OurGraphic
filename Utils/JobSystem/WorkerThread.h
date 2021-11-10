//
// Created by Glodxy on 2021/10/20.
//

#ifndef OUR_GRAPHIC_UTILS_JOBSYSTEM_WORKERTHREAD_H_
#define OUR_GRAPHIC_UTILS_JOBSYSTEM_WORKERTHREAD_H_
#include "Job.h"


namespace our_graph::job_system {
class WorkerThread {
 public:

  void Run();

 protected:
  void Yield();

  Job* GetJob();

  void Execute(Job* job);

 private:
  bool worker_thread_active_; // 是否激活了工作线程
};
}  // namespace our_graph::job_system
#endif //OUR_GRAPHIC_UTILS_JOBSYSTEM_WORKERTHREAD_H_
