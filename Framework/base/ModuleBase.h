//
// Created by Glodxy on 2021/10/25.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BASE_MODULEBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_BASE_MODULEBASE_H_

namespace our_graph {

class ModuleBase {
 protected:
  ModuleBase() noexcept = default;
  ~ModuleBase() = default;
 public:
  ModuleBase(const ModuleBase&) = delete;
  ModuleBase(ModuleBase&&) = delete;
  ModuleBase& operator=(const ModuleBase&) = delete;
  ModuleBase& operator=(ModuleBase&&) = delete;

  // 不允许new,只允许placement new
  static void* operator new(size_t, void* p) {return p;}

  // 禁用new
  static void* operator new(size_t) = delete;
  static void* operator new[](size_t) = delete;
  static void operator delete(void*) = delete;
  static void operator delete[](void*) = delete;
};

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BASE_MODULEBASE_H_
