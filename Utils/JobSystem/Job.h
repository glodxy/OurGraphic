//
// Created by Glodxy on 2021/10/19.
//

#ifndef OUR_GRAPHIC_UTILS_JOBSYSTEM_JOB_H_
#define OUR_GRAPHIC_UTILS_JOBSYSTEM_JOB_H_

#include <functional>
#include <string>
#include <atomic>

namespace our_graph::job_system {
struct Job;
using JobExec = std::function<void(Job* job, void *data)>;

struct Job {
  JobExec function;
  Job* parent; // 上一个job
  std::atomic<int32_t> unfinished_job; // 未完成的job,atomic
  char padding[]; // 此处也用来存储用户自定义数据
};

class JobQueue {
 public:
  /**
   * 只能在其他线程调用
   * 获取该queue的job
   * */
  Job* Steal();

  Job* Pop();
  void Push(Job* job);
};

Job* CreateJob(JobExec func);
Job* CreateJob(Job* parent, JobExec func);
bool IsEmptyJob(Job* job);
void Run(Job* job);
void Wait(Job* job);
bool IsJobCompleted(Job* job);
Job* GetJob();
void Execute(Job* job);
void Finish(Job* job);

/**
 * 获取当前thread的queue
 * */
JobQueue* GetWorkerThreadQueue();
}
#endif //OUR_GRAPHIC_UTILS_JOBSYSTEM_JOB_H_
