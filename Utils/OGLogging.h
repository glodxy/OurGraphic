//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_UTILS_OGLOGGING_H_
#define OUR_GRAPHIC_UTILS_OGLOGGING_H_
#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
namespace our_graph {
static std::string LOGGER_NAME;
static void InitLogger(const std::string& logger_name) {
  LOGGER_NAME = logger_name;
  std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt(logger_name);
  logger->set_error_handler([](std::string msg) {
    std::cerr<<"log error:"<<msg<<std::endl;
  });
}


template<typename ... Args>
inline void internal_log(int level, const std::string& msg,
                         Args ... args) {
  assert(level < 3 && level >= 0);
  auto logger = spdlog::get(LOGGER_NAME);
  if (level == 0) {
    logger->info(msg, args...);
  } else if (level == 1) {
    logger->warn(msg, args...);
  }
  else if(level == 2){
    logger->error(msg, args...);
  }
}

template<typename ... Args>
inline void log_info(const char* func, int line,
                const std::string& tag,
                const std::string& msg, Args... args) {
  auto logger = spdlog::get(LOGGER_NAME);
  std::string final_msg = fmt::format("[{}][{}] line {} : {}",
                                      tag, func, line, msg);
  //logger->info(std::string(func) + ":line " + std::to_string(line) + ":" + msg, args...);
  logger->info(final_msg, args...);
}

template<typename ... Args>
inline void log_warn(const char* func, int line,
                     const std::string& tag,
                     const std::string& msg, Args... args) {
  auto logger = spdlog::get(LOGGER_NAME);
  std::string final_msg = fmt::format("[{}][{}] line {} : {}",
                                      tag, func, line, msg);
  //logger->info(std::string(func) + ":line " + std::to_string(line) + ":" + msg, args...);
  logger->warn(final_msg, args...);
}

template<typename ... Args>
inline void log_error(const char* func, int line,
                      const std::string& tag,
                      const std::string& msg, Args... args) {
  auto logger = spdlog::get(LOGGER_NAME);
  std::string final_msg = fmt::format("[{}][{}] line {} : {}",
                                      tag, func, line, msg);
  //logger->info(std::string(func) + ":line " + std::to_string(line) + ":" + msg, args...);
  logger->error(final_msg, args...);
}

#ifdef _WIN32
#elif __APPLE__
#define __FUNCSIG__ __FUNCTION__
#endif

#define LOG_INFO(tag, msg, ...) log_info(__FUNCSIG__, __LINE__, tag, msg, ##__VA_ARGS__);
#define LOG_WARN(tag, msg, ...) log_warn(__FUNCSIG__, __LINE__, tag, msg, ##__VA_ARGS__);
#define LOG_ERROR(tag, msg, ...) log_error(__FUNCSIG__, __LINE__, tag, msg, ##__VA_ARGS__);
#define SIMPLE_LOG(level, msg, ...) internal_log(level, msg, ##__VA_ARGS__);
}
#endif //OUR_GRAPHIC_UTILS_OGLOGGING_H_
