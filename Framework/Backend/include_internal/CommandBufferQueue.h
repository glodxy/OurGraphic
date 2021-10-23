//
// Created by Glodxy on 2021/10/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_COMMANDBUFFERQUEUE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_COMMANDBUFFERQUEUE_H_
#include "Utils/CircularBuffer.h"
#include "Utils/Mutex.h"
#include <vector>
namespace our_graph {

class CommandBufferQueue {
  using CircularBuffer = utils::CircularBuffer;
  using Mutex = utils::Mutex;
  using Condition = utils::Condition;
  // 每一条指令的分片
  struct Slice {
    void* begin;
    void* end;
  };

 public:
  CommandBufferQueue(size_t required_size, size_t buffer_size);
  ~CommandBufferQueue();

  CircularBuffer& GetCircularBuffer() {return circular_buffer_;}

  size_t GetHightWaterMark() const {return high_water_mark_;}

  /**
   * 等待指令执行，并返回这些待执行的指令
   * */
  std::vector<Slice> WaitForCommands() const;

  // 销毁slice中的内存
  void ReleaseBuffer(const Slice& buffer);

  void Flush();

  void RequestExit();

  bool IsExitRequested() const;

 private:
  const size_t required_size_;
  CircularBuffer circular_buffer_;

  mutable Mutex lock_;
  mutable Condition condition_;
  // 待执行的指令
  mutable std::vector<Slice> command_buffer_to_exe_;
  size_t free_space_ = 0;
  size_t high_water_mark_ = 0;
  uint32_t exit_requested_ = 0;

  static constexpr uint32_t EXIT_REQUESTED = 0x31415926;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_COMMANDBUFFERQUEUE_H_
