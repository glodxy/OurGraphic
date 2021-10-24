//
// Created by Glodxy on 2021/10/24.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_DRIVER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_DRIVER_H_

#include "../include_internal/Commands.h"
// 每个command的空间，当未定义时，默认1m
#ifndef COMMAND_BUFFER_PER_SIZE_IN_MB
#define COMMAND_BUFFER_PER_SIZE_IN_MB 1
#endif

// command queue的最大command个数, 按每个command的空间 * 最大个数来计算总大小
#ifndef COMMAND_BUFFER_MAX_CNT
#define COMMAND_BUFFER_MAX_CNT 3
#endif

namespace our_graph {
  using Driver = CommandStream;

  enum class Backend : uint8_t {
    VULKAN = 1,
  };


  Driver* CreateDriver(Backend backend);
  /**
   * 刷新driver的command queue
   * 提交缓存的command buffer至待执行队列
   * */
  void FlushDriverCommand();
  void DestroyDriver(Driver* driver);
}

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_DRIVER_H_
