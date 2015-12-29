#include <mysya/ioevent/thread.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <mysya/util/exception.h>

namespace mysya {
namespace ioevent {

namespace current_thread {

__thread pid_t current_thread_id = 0;

pid_t tid() {
  if (current_thread_id == 0) {
    current_thread_id = static_cast<pid_t>(::syscall(SYS_gettid));
  }
  return current_thread_id;
}

}  // namespace current_thread

Thread::Thread()
  : tid_(0), started_(false), joined_(false) {}
Thread::~Thread() {}

void Thread::Start(const ThreadFunc &thread_func, bool joinable) {
  LockGuard lock(this->mutex_);

  if (this->started_ == true) {
    ::mysya::util::ThrowSystemErrorException(
        "Thread::Start(): thread has already started.");
  }

  this->thread_func_ = thread_func;

  pthread_attr_t attr;
  ::pthread_attr_init(&attr);

  if (joinable == false) {
    if (::pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
      ::mysya::util::ThrowSystemErrorException(
          "Thread::Start(): failed in pthread_attr_setdetachstate, strerror(%s).",
          ::strerror(errno));
    }
    this->joined_ = true;
  }

  if (::pthread_create(&this->tid_, &attr, &StartThread, this) != 0) {
    ::mysya::util::ThrowSystemErrorException(
        "Thread::Start(): failed in pthread_create, strerror(%s).",
        ::strerror(errno));
  }

  started_ = true;

  while (this->started_ == false) {
    this->cond_.Wait(this->mutex_);
  }
}

bool Thread::Join() {
  if (::pthread_equal(this->tid_, ::pthread_self())) {
    ::mysya::util::ThrowSystemErrorException(
        "Thread::Join(): thread try to join itself, strerror(%s).",
        ::strerror(errno));
  }

  LockGuard lock(this->mutex_);
  while (this->started_ == true) {
    this->cond_.Wait(this->mutex_);
  }

  if (this->joined_ == false) {
    ::pthread_join(this->tid_, NULL);
    this->joined_ = true;
    return true;
  }

  return false;
}

void Thread::Detach() {
  LockGuard lock(this->mutex_);
  if (this->joined_ == false) {
    ::pthread_detach(this->tid_);
    joined_ = true;
  }
}

bool Thread::Joinable() {
  LockGuard lock(this->mutex_);
  return this->started_ && this->joined_ == false;
}

void *Thread::StartThread(void *arg) {
  Thread *thread = static_cast<Thread *>(arg);
  {
    LockGuard lock(thread->mutex_);
    thread->started_ = true;
    thread->cond_.NotifyOne();
  }

  thread->thread_func_();

  {
    LockGuard lock(thread->mutex_);
    thread->started_ = false;
    thread->cond_.NotifyAll();
  }

  return thread;
}

}  // namespace ioevent
}  // namespace mysya
