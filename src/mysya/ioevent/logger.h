#ifndef MYSYA_IOEVENT_LOGGER_H
#define MYSYA_IOEVENT_LOGGER_H

#include <stdarg.h>

#include <functional>

#include <mysya/util/class_util.h>

namespace mysya {
namespace ioevent {

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

}  // namespace ioevent
}  // namespace mysya

#ifndef _MYSYA_BUILD_DISABLE_LOGGER_

#define MYSYA_DEBUG(content, ...) \
  ::mysya::ioevent::Logger::GetInstance()->Log(::mysya::ioevent::LogLevel::DEBUG, \
      content, ##__VA_ARGS__)
#define MYSYA_INFO(content, ...) \
  ::mysya::ioevent::Logger::GetInstance()->Log(::mysya::ioevent::LogLevel::INFO, \
      content, ##__VA_ARGS__)
#define MYSYA_WARNING(content, ...) \
  ::mysya::ioevent::Logger::GetInstance()->Log(::mysya::ioevent::LogLevel::WARNING, "%s:%d (%s) " \
      content, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MYSYA_ERROR(content, ...) \
  ::mysya::ioevent::Logger::GetInstance()->Log(::mysya::ioevent::LogLevel::ERROR, "%s:%d (%s) " \
      content, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#else  // _MYSYA_BUILD_DISABLE_LOGGER_

#define MYSYA_DEBUG(content, ...)
#define MYSYA_INFO(content, ...)
#define MYSYA_WARNING(content, ...)
#define MYSYA_ERROR(content, ...)

#endif  // _MYSYA_BUILD_DISABLE_LOGGER_

#endif  // MYSYA_IOEVENT_LOGGER_H
