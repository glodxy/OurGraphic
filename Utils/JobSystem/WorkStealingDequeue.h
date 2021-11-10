//
// Created by Glodxy on 2021/10/21.
//

#ifndef OUR_GRAPHIC_UTILS_JOBSYSTEM_WORKSTEALINGDEQUEUE_H_
#define OUR_GRAPHIC_UTILS_JOBSYSTEM_WORKSTEALINGDEQUEUE_H_

#include <cstddef>
#include <atomic>

namespace our_graph::utils {
template<class T, size_t CNT>
class WorkStealingDequeue {
  static_assert(!(CNT & (CNT - 1))); // 限制CNT为2的n次方
  static constexpr size_t MASK = CNT - 1;

 public:
  inline void Push(T item) noexcept;
  inline T Pop() noexcept;
  inline T Steal() noexcept;

  size_t GetSize() const noexcept { return CNT; }

 private:
  /**
   * 该变量在pop()/steal()中进行读写
   * */
  std::atomic<int64_t> top_ = {0};
  /**
   * 在pop()中写，在push()/steal()中读
   * */
  std::atomic<int64_t> bottom_ = {0};

  T items_[CNT];

  /**
   * 因为该类用于多线程调用，
   * 所以不能返回引用
   * */
  T GetItemAt(int64_t idx) noexcept {
    return items_[idx & MASK];
  }

  void SetItemAt(int64_t idx, T item) noexcept {
    items_[idx & MASK] = item;
  }
};

template <class T, size_t CNT>
void WorkStealingDequeue<T, CNT>::Push(T item) noexcept {
  // 此处没有顺序要求
  int64_t bottom = bottom_.load(std::memory_order_relaxed);
  SetItemAt(bottom, item);
  // 在此之前的语句不会重排到这之前
  bottom_.store(bottom + 1, std::memory_order_release);
}

template <class T, size_t CNT>
T WorkStealingDequeue<T, CNT>::Pop() noexcept {
  // 取bottom的当前值-1
  int64_t bottom = bottom_.fetch_sub(1, std::memory_order_seq_cst) - 1;

  assert(bottom >= -1);

  int64_t top = top_.load(std::memory_order_seq_cst);

  if (top < bottom) {
    return GetItemAt(bottom);
  }

  T item{};
  if (top == bottom) {
    // 说明此时是最后一个
    item = GetItemAt(bottom);

    // 此处与steal构成了竞态条件
    if (top_.compare_exchange_strong(top, top + 1,
                                     std::memory_order_seq_cst,
                                     std::memory_order_relaxed)) {
      // 如果top_为top_+1，说明此处竞争成功
      top++;
    } else {
      // 竞争失败，丢弃取到的值
      item = T();
    }
  } else {
    // 此处说明在读取到top之前便已经竞争失败。
    // 在下面进行对bottom的调整
    assert(top - bottom == 1);
  }

  bottom_.store(top, std::memory_order_relaxed);
  return item;
}

template<class T, size_t CNT>
T WorkStealingDequeue<T, CNT>::Steal() noexcept {
  while (true) {
    int64_t top = top_.load(std::memory_order_seq_cst);

    int64_t bottom = bottom_.load(std::memory_order_seq_cst);

    if (top >= bottom) {
      return T();
    }

    T item(GetItemAt(top));

    if (top_.compare_exchange_strong(top, top + 1,
                                     std::memory_order_seq_cst,
                                     std::memory_order_relaxed)) {
      return item;
    }
  }
}


}  // namespace our_graph::utils

#endif //OUR_GRAPHIC_UTILS_JOBSYSTEM_WORKSTEALINGDEQUEUE_H_
