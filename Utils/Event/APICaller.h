//
// Created by Glodxy on 2021/11/18.
//

#ifndef OUR_GRAPHIC_UTILS_EVENT_APICALLER_H_
#define OUR_GRAPHIC_UTILS_EVENT_APICALLER_H_
#include <cstdint>
#include <string>
#include <memory>
#include <map>
#include "Utils/OGLogging.h"
namespace our_graph::utils {

template <class APIInterface>
class APICaller {
  template<typename R, typename ...A>
  struct RetTraitBase {
    using return_type = R;
  };

  template<class T>
  struct RetTrait{};

  template<typename R, typename ...A>
  struct RetTrait<R(APIInterface::*)(A...)> : public RetTraitBase<R, A...> {};
 public:
  /**
   * 注册api的处理器
   * */
  static void RegisterAPIHandler(const std::string& type,
                                 uint32_t caller_id,
                                 std::weak_ptr<APIInterface> handler) {
    std::string api_caller = type + std::to_string(caller_id);

    InternalRegisterAPIHandler(api_caller, handler);
  }

  /**
   * 去除handler
   * */
  static void RemoveAPIHandler(const std::string& type,
                               uint32_t caller_id) {
    std::string api_caller = type + std::to_string(caller_id);

    InternalRemoveAPIHandler(api_caller);
  }

  template<class Func, typename ... ARGS>
  static typename RetTrait<Func>::return_type CallAPI(const std::string& type,
                      uint32_t caller_id,
                      Func func,
                      ARGS&& ...args) {
    using RetType = typename RetTrait<Func>::return_type;
    std::string api_caller = type + std::to_string(caller_id);
    auto handler_iter = handler_map_.find(api_caller);
    if (handler_iter == handler_map_.end()) {
      LOG_ERROR("APICaller", "APICaller<{}> call failed! not registed!", api_caller);
      return RetType();
    }

    std::weak_ptr<APIInterface> weak_handler = handler_map_[api_caller];
    auto strong_this = weak_handler.lock();
    if (strong_this == nullptr) {
      LOG_ERROR("APICaller", "APICaller<{}> call failed! handler released!", api_caller);
      return RetType();
    }
    auto obj = strong_this.get();
    return (obj->*func)(std::forward<ARGS>(args)...);
  }

 private:


  static void InternalRegisterAPIHandler(const std::string& caller_id,
                                         std::weak_ptr<APIInterface> handler) {
    if (handler_map_.find(caller_id) != handler_map_.end()) {
      // 已注册，直接返回
      return;
    }
    handler_map_[caller_id] = handler;
  }

  static void InternalRemoveAPIHandler(const std::string& caller_id) {
    if (handler_map_.find(caller_id) == handler_map_.end()) {
      return;
    }
    handler_map_.erase(caller_id);
  }


  static std::map<std::string, std::weak_ptr<APIInterface>> handler_map_;
};

template<class APIInterface>
std::map<std::string, std::weak_ptr<APIInterface>> APICaller<APIInterface>::handler_map_;

}
#endif //OUR_GRAPHIC_UTILS_EVENT_APICALLER_H_
