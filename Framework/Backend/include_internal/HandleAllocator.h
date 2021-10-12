//
// Created by Glodxy on 2021/10/8.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_HANDLEALLOCATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_HANDLEALLOCATOR_H_

#include <unordered_map>
#include <string>
#include <mutex>
#include "../include/Handle.h"
#include "Utils/OGLogging.h"

namespace our_graph {
class HandleAllocator {
 public:
  static HandleAllocator& Get() {
    static HandleAllocator allocator;
    return allocator;
  }

  /**
   * 分配且构造
   * */
  template<typename T, typename  ... ARGS>
  Handle<T> AllocateAndConstruct(ARGS&& ... args) noexcept {
    Handle<T> handle{AllocateHandle(sizeof(T))};
    T* addr = HandleCast<T*>(handle);
    new(addr) T(std::forward<ARGS>(args)...);
    type_lock_.lock();
    type_name_[handle.GetId()] = typeid(T).name();
#if DEBUG
    LOG_INFO("HandleAllocator", "Create type:{}, handle:{}",
             type_name_[handle.GetId()], handle.GetId());
#endif
    type_lock_.unlock();
    return handle;
  }

  /**
   * 仅构造
   * */
  template<typename T, typename  ... ARGS>
  Handle<T> Allocate() noexcept {
    Handle<T> handle{AllocateHandle(sizeof(T))};
    T* addr = HandleCast<T*>(handle);
    type_lock_.lock();
    type_name_[handle.GetId()] = typeid(T).name();
    type_lock_.unlock();
    return handle;
  }

  /**
   * 重新构造
   * */
  template<typename T, typename B, typename ...ARGS>
  typename std::enable_if_t<std::is_base_of_v<B, T>, T>*
  DestroyAndConstruct(const Handle<B>& handle, ARGS&& ...args) noexcept {
    T* addr = HandleCast<T*>(const_cast<Handle<B>&>(handle));

    addr->~T();
    new(addr) T(std::forward<ARGS>(args)...);

    type_lock_.lock();
    type_name_[handle.GetId()] = typeid(T).name();
    type_lock_.unlock();

    return addr;
  }

  /**
   * 仅构造
   * */
  template<typename T, typename B, typename ...ARGS>
  typename std::enable_if_t<std::is_base_of_v<B, T>, T>*
  Construct(const Handle<B>& handle, ARGS&& ...args) noexcept {
    T* addr = HandleCast<T*>(const_cast<Handle<B>&>(handle));

    new(addr) T(std::forward<ARGS>(args)...);

    type_lock_.lock();
    type_name_[handle.GetId()] = typeid(T).name();
    type_lock_.unlock();

    return addr;
  }

  template<typename B, typename T,
      typename = typename std::enable_if_t<std::is_base_of_v<B, T>, T>>
  void Deallocate(const Handle<B>& handle, const T* p) noexcept {
    if (!handle) {
      return;
    }
    type_lock_.lock();
    if (type_name_.find(handle.GetId()) == type_name_.end()) {
      type_lock_.unlock();
      return;
    }
    type_lock_.unlock();
    if (p) {
      std::string type_id = "";
      type_lock_.lock();
      type_id = type_name_[handle.GetId()];
#if DEBUG
      LOG_WARN("HandleAllocator", "try to deallocator class:{}, handle:{}",
               type_id, handle.GetId());
#endif
      type_name_.erase(handle.GetId());
      type_lock_.unlock();

      if (type_id != std::string(typeid(T).name())) {
        LOG_ERROR("HandleAllocator", "Deallocate Failed! type get:{}, "
                                     "type input:{}, "
                                     "handle:{}",
                                     type_id, typeid(T).name(),
                                     handle.GetId());
        std::terminate();
      }
      handle.object_ = handle.NULL_HANDLE;
      p->~T();
    }
  }

  template<typename T>
  void Deallocate(Handle<T>& handle) noexcept {
    const T* p = HandleCast<const T*>(handle);
    Deallocate(handle, p);
  }


  /**
   * 将句柄转换为指针
   * */
  template<typename P_T, typename B>
  inline typename std::enable_if_t<std::is_pointer_v<P_T> &&
      std::is_base_of_v<B, typename std::remove_pointer_t<P_T>>, P_T>
  HandleCast(Handle<B>& handle) noexcept {
    void* const p = HandleToPointer(handle.GetId());
    return static_cast<P_T>(p);
  }

  template<typename P_T, typename B>
  inline typename std::enable_if_t<std::is_pointer_v<P_T> &&
      std::is_base_of_v<B, typename std::remove_pointer_t<P_T>>, P_T>
  HandleCast(const Handle<B>& handle) noexcept {
    return HandleCast<P_T>(const_cast<Handle<B>&>(handle));
  }

 private:
  HandleAllocator() = default;

  //todo : 使用内存池优化
  //todo: 更换id生成方式
  HandleBase::HandleId AllocateHandle(size_t size) noexcept {
    void* p = ::malloc(size);

    std::unique_lock lock(type_lock_);
    size_t id = ++id_;
    memory_.emplace(id, p);
    lock.unlock();

    return id;
  }

  //todo:与allocate配套修改
  void DeallocateHandle(HandleBase::HandleId id, size_t size) noexcept {
    void* p = nullptr;
    std::unique_lock lock(type_lock_);
    p = memory_[id];
    memory_.erase(id);
    lock.unlock();

    ::free(p);
  }

  inline void* HandleToPointer(HandleBase::HandleId id) const noexcept {
    std::lock_guard lock(type_lock_);
    auto res = memory_.find(id);
    if (res != memory_.end()) {
      return res->second;
    }
    return nullptr;
  }

  int id_ = 0;
  std::unordered_map<HandleBase::HandleId, void*> memory_;
  mutable std::mutex type_lock_;
  mutable std::unordered_map<HandleBase::HandleId, std::string> type_name_;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_HANDLEALLOCATOR_H_
