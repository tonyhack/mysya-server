#include <assert.h>
#include <stdio.h>

#include <unistd.h>

#include <vector>

#include "masya/logger.h"
#include "masya/mutex.h"
#include "masya/thread.h"

namespace masya {
namespace test {

void NativeLoggerTestFunc() {
  MASYA_DEBUG("Logger test1.");
  MASYA_INFO("Logger test2.");
  MASYA_WARNING("Logger test3.");
  MASYA_ERROR("Logger test4.");
}


void ThreadFunc() {
  for (size_t i = 0; i < 9999; ++i) {
    MASYA_DEBUG("Logger test1, thread_id=%d", ::pthread_self());
    MASYA_INFO("Logger test2, thread_id=%d.", ::pthread_self());
    MASYA_WARNING("Logger test3, thread_id=%d.", ::pthread_self());
    MASYA_ERROR("Logger test4, thread_id=%d.", ::pthread_self());
  }
}

class LoggerFile {
 public:
  LoggerFile(const std::string &file) {
    FILE *fp = ::fopen(file.c_str(), "a");
    assert(NULL != fp);
    ::setbuf(fp, NULL);
    this->fp_ = fp;
  }
  ~LoggerFile() {
    if (this->fp_ != NULL) {
      ::fclose(this->fp_);
    }
  }

  void Log(int level, const char *format, va_list args) {
    if (level < LogLevel::MIN || level >= LogLevel::MAX) {
      return;
    }

    static const char *log_level_string[] = {
      "DEBUG", "INFO", "WARNING", "ERROR", };

    LockGuard lock(this->mutex_);

    ::fprintf(this->fp_, "[%s] ", log_level_string[level]);
    ::vfprintf(this->fp_, format, args);
    ::fprintf(this->fp_, "\n");
  }

 private:
  Mutex mutex_;
  FILE *fp_;
};

void MultiThreadTestFunc(size_t thread_num) {
  LoggerFile logger_file("./log.txt");

  Logger::GetInstance()->SetLogCallback(std::bind(&LoggerFile::Log, &logger_file,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  std::vector<Thread *> threads;

  for (size_t i = 0; i < thread_num; ++i) {
    Thread *thread = new Thread();
    threads.push_back(thread);
    thread->Start(std::bind(ThreadFunc));
  }

  for (size_t i = 0; i < thread_num; ++i) {
    threads[i]->Join();
  }
}

}  // namespace test
}  // namespace masya

int main(int argc, char *argv[]) {
  ::masya::test::NativeLoggerTestFunc();
  ::masya::test::MultiThreadTestFunc(10);

  return 0;
}

