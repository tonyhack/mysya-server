#include <mysya/ioevent/logger.h>

#include <stdio.h>

#include <mysya/ioevent/mutex.h>

namespace mysya {
namespace ioevent {

static void DefaultLogFunc(int level, const char *format, va_list args) {
  if (level < LogLevel::MIN || level >= LogLevel::MAX) {
    return;
  }

  static const char *log_level_string[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
  };

  static Mutex mutex;
  LockGuard lock(mutex);

  ::fprintf(stderr, "[%s] ", log_level_string[level]);
  ::vfprintf(stderr, format, args);
  ::fprintf(stderr, "\n");
}

Logger::Logger() :
  log_cb_(DefaultLogFunc) {}
Logger::~Logger() {}

void Logger::SetLogCallback(const LogCallback &cb) {
  this->log_cb_ = cb;
}

void Logger::Log(int level, const char *format, ...) {
  va_list args;
  va_start(args, format);
  log_cb_(level, format, args);
  va_end(args);
}

}  // namespace ioevent
}  // namespace mysya

