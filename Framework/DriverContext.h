//
// Created by Glodxy on 2021/9/18.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_DRIVERCONTEXT_H_
#define OUR_GRAPHIC_FRAMEWORK_DRIVERCONTEXT_H_


namespace our_graph {
/**
 * 该结构体为外内部连接使用的上下文，外部通过该结构体将数据传入内部
 * */
class DriverContext {
 public:
  static DriverContext& Get() {
    static DriverContext context;
    return context;
  }
  void* window_instance_ {nullptr};
  void* window_handle_ {nullptr};

 private:
  DriverContext() = default;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_DRIVERCONTEXT_H_
