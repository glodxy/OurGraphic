//
// Created by Glodxy on 2021/10/19.
//

#include "Job.h"


namespace our_graph::job_system {

Job* AllocateJob() {
  return new Job();
}

Job* CreateJob(JobExec func) {
  Job* job = AllocateJob();
  job->function = func;
  job->parent = nullptr;
  job->unfinished_job = 1;

  return job;
}

Job* CreateJob(Job* parent, JobExec func) {
  parent->unfinished_job ++;

  Job* job = AllocateJob();
  job->function = func;
  job->parent = parent;
  job->unfinished_job = 1;

  return job;
}

void Run(Job* job) {
  JobQueue* queue = GetWorkerThreadQueue();
  queue->Push(job);
}

void Wait(Job* job) {
  // 当前未执行完成则一直执行
  while (!IsJobCompleted(job)) {
    Job* next_job = GetJob();
    if (next_job) {
      Execute(next_job);
    }
  }
}

Job* GetJob() {
  // todo:获取自身的queue
  JobQueue* queue = GetWorkerThreadQueue();
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

void Execute(Job* job) {
  job->function(job, job->data);
  Finish(job);
}

void Finish(Job* job) {
  const uint32_t unfinished_jobs =
      (--job->unfinished_job);
  if (unfinished_jobs == 0) {
    // 都已完成
    // todo:atomic增加待删除数，并加入删除队列

    if (job->parent) {
      Finish(job->parent);
    }
  }
}

}