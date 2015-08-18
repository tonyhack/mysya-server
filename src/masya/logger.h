#ifndef MASYA_LOGGER_H
#define MASYA_LOGGER_H

#include <stdarg.h>

#include <functional>

#include "masya/class_util.h"

namespace masya {

struct LogLevel {
  enum type {
    MIN = 0,

    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,

    MAX,
  };
};

class Logger {
 public:
  typedef std::function<void (int, const char *, va_list)> LogCallback;

  void SetLogCallback(const LogCallback &cb);
  void Log(int level, const char *format, ...);

 private:
  MASYA_SINGLETON2(Logger);

  LogCallback log_cb_;
};

}  // namespace masya

#ifndef _MASYA_BUILD_DISABLE_LOGGER_

#define MASYA_DEBUG(content, ...) \
  ::masya::Logger::GetInstance()->Log(::masya::LogLevel::DEBUG, \
      content, ##__VA_ARGS__)
#define MASYA_INFO(content, ...) \
  ::masya::Logger::GetInstance()->Log(::masya::LogLevel::INFO, \
      content, ##__VA_ARGS__)
#define MASYA_WARNING(content, ...) \
  ::masya::Logger::GetInstance()->Log(::masya::LogLevel::WARNING, "%s:%d (%s) " \
      content, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MASYA_ERROR(content, ...) \
  ::masya::Logger::GetInstance()->Log(::masya::LogLevel::ERROR, "%s:%d (%s) " \
      content, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#else  // _MASYA_BUILD_DISABLE_LOGGER_

#define MASYA_DEBUG(content, ...)
#define MASYA_INFO(content, ...)
#define MASYA_WARNING(content, ...)
#define MASYA_ERROR(content, ...)

#endif  // _MASYA_BUILD_DISABLE_LOGGER_

#endif  // MASYA_LOGGER_H
