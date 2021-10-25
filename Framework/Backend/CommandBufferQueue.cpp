//
// Created by Glodxy on 2021/10/23.
//
#include <cassert>
#include <mutex>
#include "include_internal/CommandBufferQueue.h"
#include "include_internal/Commands.h"
#include "Utils/OGLogging.h"
namespace our_graph {
CommandBufferQueue::CommandBufferQueue(size_t required_size, size_t buffer_size)
  : required_size_((required_size + CircularBuffer::BLOCK_MASK) & ~CircularBuffer::BLOCK_MASK),
    circular_buffer_(buffer_size),
    free_space_(circular_buffer_.Size()) {
  assert(circular_buffer_.Size() > required_size);
}

CommandBufferQueue::~CommandBufferQueue() {
  // 析构前需要先清空
  assert(command_buffer_to_exe_.empty());
}

void CommandBufferQueue::RequestExit() {
  std::lock_guard<Mutex> lock(lock_);
  exit_requested_ = EXIT_REQUESTED;
  condition_.notify_one();
}

bool CommandBufferQueue::IsExitRequested() const {
  std::lock_guard<Mutex> lock(lock_);
  if (exit_requested_ != 0 && exit_requested_ != EXIT_REQUESTED) {
    LOG_ERROR("CommandBufferQueue", "exit request error:{}", exit_requested_);
    assert(exit_requested_ == 0 || exit_requested_ == EXIT_REQUESTED);
  }
  return (bool)exit_requested_;
}

/**
 * 该函数的作用是在当前的command queue末尾添加一个NoopCommand标识执行的结束
 * 因为执行是使用的是每个Command的execute函数来得到下一个command的位置
 * */
void CommandBufferQueue::Flush() {
  CircularBuffer& circular_buffer = circular_buffer_;
  if (circular_buffer.Empty()) {
    return;
  }
  // 添加一个空指令
  new(circular_buffer.Allocate(sizeof(NoopCommand))) NoopCommand(nullptr);

  void* const head = circular_buffer.GetHead();
  void* const tail = circular_buffer.GetTail();

  uint32_t used_size = uint32_t(intptr_t(head) - intptr_t(tail));

  circular_buffer.Circularize();

  std::unique_lock<Mutex> lock(lock_);
  command_buffer_to_exe_.push_back({tail, head});

  if (used_size > free_space_) {
    // 如果要使用的一个slice的size大于剩余的可用size
    LOG_ERROR("CommandBufferQueue", "buffer overflow! increase the size! required this time:{}", used_size);
    assert(used_size <= free_space_);
  }

  free_space_ -= used_size;
  const size_t required_size = required_size_;

  condition_.notify_one();
  // 剩余空间不够大时，等待
  if (free_space_ < required_size) {
    condition_.wait(lock, [this, required_size]() ->bool {
      return free_space_ >= required_size;
    });
  }
}

std::vector<CommandBufferQueue::Slice> CommandBufferQueue::WaitForCommands() const {
  std::unique_lock<Mutex> lock(lock_);
  uint64_t before = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
  ).count();
  // 无command的时候等待
  while (command_buffer_to_exe_.empty() && !exit_requested_) {
    condition_.wait(lock);
  }
  uint64_t end = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
  ).count();
  LOG_ERROR("CommandQueue", "wait cost:{}", (end - before));

  if (exit_requested_ != 0 && exit_requested_ != EXIT_REQUESTED) {
    LOG_ERROR("CommandBufferQueue", "exit_request is corrupted:{}", exit_requested_);
    assert(exit_requested_ == 0 || exit_requested_ == EXIT_REQUESTED);
  }

  return std::move(command_buffer_to_exe_);
}

void CommandBufferQueue::ReleaseBuffer(const Slice &buffer) {
  std::lock_guard<Mutex> lock(lock_);
  free_space_ += uintptr_t (buffer.end) - uintptr_t(buffer.begin);
  condition_.notify_one();
}
}