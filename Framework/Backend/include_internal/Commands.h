//
// Created by Glodxy on 2021/10/22.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_COMMANDS_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_COMMANDS_H_
#include <cstddef>
#include <tuple>
#include <functional>
#include <cassert>
#include "Utils/CircularBuffer.h"
#include "../include/Driver.h"
#include "Utils/MacroUtils.h"

namespace our_graph {
class DriverApi;
class CommandBase;
class DispatcherBase {
 public:
  using Executor = void(*)(DriverApi& driver, CommandBase* self, intptr_t* next);

  Executor BeginFrame_;
  Executor EndFrame_;
  Executor Tick_;
  Executor Flush_;
  Executor Finish_;
  /************异步返回***********/
  Executor CreateVertexBuffer_;
};


class CommandBase {
  static constexpr size_t OBJECT_ALIGNMENT = alignof(std::max_align_t);

 protected:
  using Executor = DispatcherBase::Executor;
  constexpr explicit CommandBase(Executor exe) noexcept : execute_(exe) {

  }

 public:
  // 获取对齐的字节数
  static constexpr size_t Align(size_t v) {
    return (v + (OBJECT_ALIGNMENT - 1)) & (-OBJECT_ALIGNMENT);
  }

  // 执行该command
  inline CommandBase* Execute(DriverApi& driver) {
    intptr_t next;
    execute_(driver, this, &next);
    // 根据偏移计算下一个command的位置
    return reinterpret_cast<CommandBase*>(reinterpret_cast<intptr_t>(this) + next);
  }

  inline ~CommandBase() noexcept = default;
 private:
  Executor execute_;
};


/**
 * 这三个函数的作用是转发参数，并调用Driver中对应的函数
 * invoke：此处判断了method与driver的对应关系，确保driver中存在该method，然后将参数直接转发
 * trampoline：此处将传入的参数解包，解为参数列表
 * apply：此处将传入的参数类（tuple）进行拆分，按照顺序组成seq
 * */
template<typename T, typename Type, typename D, typename ... ARGS>
constexpr decltype(auto) invoke(Type T::* m, D&& d, ARGS&& ... args) {
  static_assert(std::is_base_of<T, std::decay_t<D>>::value,
                "member function and object not related");
  return (std::forward<D>(d).*m)(std::forward<ARGS>(args)...);
}

template<typename M, typename D, typename T, std::size_t... I>
constexpr decltype(auto) trampoline(M&& m, D&& d, T&& t, std::index_sequence<I...>) {
  return invoke(std::forward<M>(m), std::forward<D>(d), std::get<I>(std::forward<T>(t))...);
}

template<typename M, typename D, typename T>
constexpr decltype(auto) apply(M&& m, D&& d, T&& t) {
  return trampoline(std::forward<M>(m), std::forward<D>(d), std::forward<T>(t),
                    std::make_index_sequence< std::tuple_size<std::remove_reference_t<T>>::value >{});
}

/**
 * 该类用于包装DriverApi中的method
 * */
template<typename... ARGS>
struct CommandType;

/**
 * 外面的这个类用于匹配DriverApi中的method
 * */
template<typename ...ARGS>
struct CommandType<void (DriverApi::*)(ARGS...)> {
  template<void(DriverApi::*)(ARGS...)>
  class Command : public CommandBase {
    using LocalParameters = std::tuple<std::remove_reference_t<ARGS>...>;
    LocalParameters  args_;

   public:
    template<typename M, typename D>
    static inline void Execute(
        M&& method, D&& driver,
        CommandBase* base, intptr_t* next) noexcept {
      Command* self = static_cast<Command*>(base);
      *next = Align(alignof(Command));

      apply(std::forward<M>(method), std::forward<D>(driver), std::move(self->args_));
      self->~Command();
    }

    inline Command(Command&&) = default;

    template<typename ... A>
    inline explicit constexpr Command(Executor executor, A&&...args)
      : CommandBase(executor) , args_(std::forward<A>(args)...) {
    }

    // 此处重载new防止编译器的检查，因为使用placement new
    inline void* operator new(std::size_t size, void* ptr) {
      return ptr;
    }
  };

};

#define COMMAND_TYPE(method) CommandType<decltype(&DriverApi::method)>::Command<&DriverApi::method>

class CustomCommand : public CommandBase {
  std::function<void()> command_;
  static void Execute(DriverApi&, CommandBase* base, intptr_t* next) noexcept;
 public:
  inline CustomCommand(CustomCommand&& rhs) = default;
  inline explicit CustomCommand(std::function<void()> cmd)
      : CommandBase(Execute), command_(std::move(cmd)) { }
};

class NoopCommand : public CommandBase {
  intptr_t next_;
  static void Execute(DriverApi&, CommandBase* self, intptr_t* next) noexcept {
    *next = static_cast<NoopCommand*>(self)->next_;
  }
 public:
  inline constexpr explicit NoopCommand(void* next) noexcept
      : CommandBase(Execute), next_(size_t((char *)next - (char *)this)) { }
};


class CommandStream {
  DriverApi* driver_ = nullptr;
  utils::CircularBuffer* current_buffer_ = nullptr;
  DispatcherBase* dispatcher_ = nullptr;
  bool use_performance_counter = false;

 public:
#define DECL_CMD_N(methodName, ...)  {\
  using Cmd = COMMAND_TYPE(methodName); \
  void* const p = AllocateCommand(CommandBase::Align(sizeof(Cmd))); \
  new(p) Cmd(dispatcher_->methodName##_, APPLY(std::move, ##__VA_ARGS__));   \
}

#define DECL_CMD(methodName)  {\
  using Cmd = COMMAND_TYPE(methodName); \
  void* const p = AllocateCommand(CommandBase::Align(sizeof(Cmd))); \
  new(p) Cmd(dispatcher_->methodName##_);   \
}

#define DECL_CMD_RETURN_N(RetType, methodName, ...) { \
  RetType res = driver_->methodName##S();             \
  using Cmd = COMMAND_TYPE(methodName##R);            \
  void* const p = AllocateCommand(CommandBase::Align(sizeof(Cmd))); \
  new(p) Cmd(dispatcher_->methodName##_, RetType(res), APPLY(std::move, ##__VA_ARGS__)); \
  return res;\
}

#define DECL_CMD_RETURN(RetType, methodName) { \
  RetType res = driver_->methodName##S();             \
  using Cmd = COMMAND_TYPE(methodName##R);            \
  void* const p = AllocateCommand(CommandBase::Align(sizeof(Cmd))); \
  new(p) Cmd(dispatcher_->methodName_, RetType(res)); \
  return res;\
}

  //todo:声明接口
  /**************普通异步接口*******************/
  void BeginFrame(int64_t time_ns,
                  uint32_t frame_id) {
    DECL_CMD_N(BeginFrame, time_ns, frame_id);
  }

  void EndFrame(uint32_t frame_id) {
    DECL_CMD_N(EndFrame, frame_id);
  }

  void Tick() {
    DECL_CMD(Tick);
  }

  void Flush() {
    DECL_CMD(Flush);
  }

  void Finish() {
    DECL_CMD(Finish);
  }

  /***************同步接口********************/
  void Init(std::unique_ptr<IPlatform> platform) {
    driver_->Init(std::move(platform));
  }

  /***************带返回的异步接口***************/
  VertexBufferHandle CreateVertexBuffer(uint8_t buffer_count,
                                        uint8_t attribute_count,
                                        uint32_t vertex_count,
                                        AttributeArray attributes) {
    DECL_CMD_RETURN_N(VertexBufferHandle, CreateVertexBuffer,
                      buffer_count, attribute_count, vertex_count,
                      attributes);
  }
 public:
  CommandStream() noexcept = default;
  CommandStream(DriverApi& driver, utils::CircularBuffer& buffer) noexcept;

  /**
   * @param buffer:执行的command的头指针
   * */
  void Execute(void * buffer);

  void QueueCommand(std::function<void()> command);

  /**
   * 在当前commandbuffer的基础上分配内存空间，
   * 会构建
   * 在command调用后会自动释放
   * 但不会调用析构函数
   * */
  inline void* Allocate(size_t size, size_t alignment = 8) noexcept {
    assert(alignment && !(alignment & (alignment-1)));

    const size_t s = CustomCommand::Align(sizeof(NoopCommand) + size + alignment - 1);

    // 分配空间并插入空command
    char* const p = (char*) AllocateCommand(s);
    new(p) NoopCommand(p + s);

    // 找到user 自定义data区
    void* data = (void*)((uintptr_t(p) + sizeof(NoopCommand) + alignment - 1) & ~(alignment - 1));
    assert(data >= p + sizeof(NoopCommand));

    return data;
  }

  template<class PodType, typename = typename std::enable_if<std::is_trivially_destructible<PodType>::value>::type>
  inline PodType* AllocatePod(size_t count = 1,
                              size_t alignment = alignof(PodType)) noexcept {
    return static_cast<PodType*>(Allocate(count * sizeof(PodType), alignment));
  }

 private:
  inline void* AllocateCommand(size_t size) {
    return current_buffer_->Allocate(size);
  }
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_COMMANDS_H_
