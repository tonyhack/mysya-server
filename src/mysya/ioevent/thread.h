#ifndef MYSYA_IOEVENT_THREAD_H
#define MYSYA_IOEVENT_THREAD_H

#include <functional>

#include <mysya/ioevent/condition_variable.h>
#include <mysya/ioevent/mutex.h>
#include <mysya/util/class_util.h>

namespace mysya {
namespace ioevent {

class Thread {
 public:
  typedef std::function<void ()> ThreadFunc;

  Thread();
  ~Thread();

  void Start(const ThreadFunc &thread_func, bool joinable = true);
  bool Join();
  void Detach();

  bool Joinable();

 private:
  static void *StartThread(void *arg);

  pthread_t tid_;
  ThreadFunc thread_func_;
  Mutex mutex_;
  ConditionVariable cond_;

  bool started_;
  bool joined_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Thread);
};

template <class ThreadType>
class ThreadGuardTemplate {
 public:
  explicit ThreadGuardTemplate(ThreadType &thread)
    : thread_(thread) {
  }

  ~ThreadGuardTemplate() {
    if (this->thread_.joinable()) {
      this->thread_.join();
    }
  }

 private:
  ThreadType thread_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(ThreadGuardTemplate<ThreadType>);
};

typedef ThreadGuardTemplate<Thread> ThreadGuard;

namespace current_thread {

pid_t tid();

}  // namespace current_thread

}  // namespace ioevent
}  // namespace mysya

#endif  // MYSYA_IOEVENT_THREAD_H
