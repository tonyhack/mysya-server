#ifndef MYSYA_LOGGER_H
#define MYSYA_LOGGER_H

#include <stdarg.h>

#include <functional>

#include <mysya/class_util.h>

namespace mysya {

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
  MYSYA_SINGLETON2(Logger);

  LogCallback log_cb_;
};

}  // namespace mysya

#ifndef _MYSYA_BUILD_DISABLE_LOGGER_

#define MYSYA_DEBUG(content, ...) \
  ::mysya::Logger::GetInstance()->Log(::mysya::LogLevel::DEBUG, \
      content, ##__VA_ARGS__)
#define MYSYA_INFO(content, ...) \
  ::mysya::Logger::GetInstance()->Log(::mysya::LogLevel::INFO, \
      content, ##__VA_ARGS__)
#define MYSYA_WARNING(content, ...) \
  ::mysya::Logger::GetInstance()->Log(::mysya::LogLevel::WARNING, "%s:%d (%s) " \
      content, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MYSYA_ERROR(content, ...) \
  ::mysya::Logger::GetInstance()->Log(::mysya::LogLevel::ERROR, "%s:%d (%s) " \
      content, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#else  // _MYSYA_BUILD_DISABLE_LOGGER_

#define MYSYA_DEBUG(content, ...)
#define MYSYA_INFO(content, ...)
#define MYSYA_WARNING(content, ...)
#define MYSYA_ERROR(content, ...)

#endif  // _MYSYA_BUILD_DISABLE_LOGGER_

#endif  // MYSYA_LOGGER_H
