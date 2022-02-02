//
// Created by Glodxy on 2021/11/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMENUM_H_
#define OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMENUM_H_
#include <cstddef>
#include <cstdint>
namespace our_graph {
constexpr static const char* SYSTEM_CALLER = "System";
enum class SystemID : uint32_t {
  NONE = 0,
  CAMERA = 1, //camera system 的id
  RENDER = 2, //用于渲染的system
  LIGHT = 3, // 管理光照的system
};

}

#endif //OUR_GRAPHIC_FRAMEWORK_MANAGER_SYSTEMENUM_H_
