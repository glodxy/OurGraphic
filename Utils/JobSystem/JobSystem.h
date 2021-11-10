//
// Created by Glodxy on 2021/10/19.
//

#ifndef OUR_GRAPHIC_UTILS_JOBSYSTEM_JOBSYSTEM_H_
#define OUR_GRAPHIC_UTILS_JOBSYSTEM_JOBSYSTEM_H_

#include <functional>
#include "WorkStealingDequeue.h"

constexpr size_t CACHELINE_SIZE = 64;

namespace our_graph::job_system {
class JobSystem {
  static constexpr size_t MAX_JOB_CNT = 16384;
  static_assert(MAX_JOB_CNT);

  using WorkerQueue = utils::WorkStealingDequeue<uint16_t , MAX_JOB_CNT>;

 public:
  class Job;

  using JobFunc = void(*)(void*, JobSystem&, Job*);

  class alignas(CACHELINE_SIZE) Job {
   public:
    Job() noexcept {}
    Job(const Job&) = delete;
    Job(Job&&) = delete;

   private:
    friend class JobSystem;

    static constexpr size_t JOB_STORAGE_SIZE_BYTES =
        sizeof(std::function<void()>) > 48 ? sizeof(std::function<void()>) : 48;
    static constexpr size_t JOB_STORAGE_SIZE_WORDS =
        (JOB_STORAGE_SIZE_BYTES + (sizeof(void*)) - 1) / sizeof(void*);

    void* storage_[JOB_STORAGE_SIZE_WORDS]; // 每个存储占一个字
    JobFunc function_;
    uint16_t parent_;
    std::atomic<uint16_t> running_job_cnt_ = {1}; // 剩余job数
    mutable std::atomic<uint16_t> ref_cnt_ = {1}; // 该jop的引用计数
  };

  /**
   * @param thread_cnt
   * @param adoptable_thread_cnt
   * */
  explicit JobSystem(size_t thread_cnt = 0, size_t adoptable_thread_cnt = 1) noexcept;

  ~JobSystem();

  Job* SetRootJob(Job* job) noexcept {
    return root_job_ = job;
  }

  Job* Create(Job* parent, JobFunc func) noexcept;

  // 创建空job
  Job* CreateJob(Job* parent = nullptr) noexcept {
    return Create(parent, nullptr);
  }

  template<class T, void(T::*method)(JobSystem&, Job*)>
  Job* CreateJob(Job* parent, T* data) noexcept {
    Job* job = Create(parent, [](void* user, JobSystem& js, Job* job) {
      (*static_cast<T**>(user)->*method)(js, job);
    });
    if (job) {
      job->storage[0] = data;
    }
    return job;
  }

  template<class T, void(T::*method)(JobSystem&, Job*)>
  Job* CreateJob(Job* parent, T data) noexcept {
    static_assert(sizeof(data) <= sizeof(Job::storage_), "user data too large");
    Job* job = Create(parent, [](void* user, JobSystem& js, Job* job) {
      T* that = static_cast<T*>(user);
      (that->*method)(js, job);
      that->~T();
    });
    if (job) {
      // placement new 导入数据
      new(job->storage_) T(std::move(data));
    }
    return job;
  }

  template<class T>
  Job* CreateJob(Job* parent, T functor) noexcept {
    static_assert(sizeof(functor) <= sizeof(Job::storage_), "functor too large");
    Job* job = Create(parent, [](void* user, JobSystem& js, Job* job) {
      T& that = *static_cast<T*>(user);
      that(js, job);
      that.~T();
    });
    if (job) {
      new(job->storage_) T(std::move(functor));
    }
    return job;
  }


  /**
   * 取消还没Run的job
   * */
  void Cancel(Job*& job) noexcept''
 private:
  Job* root_job_;
};
}  // namespace our_graph::job_system

#endif //OUR_GRAPHIC_UTILS_JOBSYSTEM_JOBSYSTEM_H_
