//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_UTILS_OGLOGGING_H_
#define OUR_GRAPHIC_UTILS_OGLOGGING_H_
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
namespace our_graph {
#ifdef DEBUG
const static auto LOGGER = spdlog::stderr_color_mt("logger");
#else
const static auto LOGGER = spdlog::basic_logger_mt("logger", "log.txt");
#endif

template<typename ... Args>
inline void log_info(const char* func, int line,
                const std::string& msg, Args... args) {
  LOGGER->info(std::string(func) + ":line " + std::to_string(line) + ":" + msg, args...);
}

template<typename ... Args>
inline void log_warn(const char* func, int line,
                     const std::string& msg, Args... args) {
  LOGGER->warn(std::string(func) + ":line " + std::to_string(line) + ":" + msg, args...);
}

template<typename ... Args>
inline void log_error(const char* func, int line,
                      const std::string& msg, Args... args) {
  LOGGER->error(std::string(func) + ":line " + std::to_string(line) + ":" + msg, args...);
}
#define LOG_INFO(tag, msg, ...) log_info(__FUNCSIG__, __LINE__, "[" tag "]"##msg, ##__VA_ARGS__);
#define LOG_WARN(tag, msg, ...) log_warn(__FUNCSIG__, __LINE__, "[" tag "]"##msg, ##__VA_ARGS__);
#define LOG_ERROR(tag, msg, ...) log_error(__FUNCSIG__, __LINE__, "[" tag "]"##msg, ##__VA_ARGS__);

}
#endif //OUR_GRAPHIC_UTILS_OGLOGGING_H_
