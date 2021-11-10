//
// Created by Glodxy on 2021/10/20.
//

#include "WorkerThread.h"

namespace our_graph::job_system {

void WorkerThread::Run() {
  while (worker_thread_active_) {
    Job* job = GetJob();
    if (job) {
      Execute(job);
    }
  }
}

Job *WorkerThread::GetJob() {
  // todo:获取自身的queue
  JobQueue* queue;
  Job* job = queue->Pop();
  if (IsEmptyJob(job)) {
    //todo:随机获取目标queue
    JobQueue* steal_queue;
    if (steal_queue == queue) {
      Yield();
      return nullptr;
    }

    Job* stolen_job = steal_queue->Steal();
    if (IsEmptyJob(stolen_job)) {
      Yield();
      return nullptr;
    }

    return stolen_job;
  }

  return job;
}

void WorkerThread::Execute(Job *job) {
  job->function(job, job->data);
}

}