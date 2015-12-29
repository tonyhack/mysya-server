#ifndef MYSYA_IOEVENT_CONDITION_VARIABLE_H
#define MYSYA_IOEVENT_CONDITION_VARIABLE_H

#include <pthread.h>
#include <string.h>

#include <mysya/ioevent/mutex.h>
#include <mysya/util/exception.h>

namespace mysya {
namespace ioevent {

class ConditionVariable {
 public:
   ConditionVariable() {
     if (::pthread_cond_init(&this->cond_, NULL) != 0) {
       ::mysya::util::ThrowSystemErrorException(
           "ConditionVariable::ConditionVariable() failed in pthread_cond_init, strerror(%s).",
          ::strerror(errno));
     }
  }
  ~ConditionVariable() {
    ::pthread_cond_destroy(&this->cond_);
  }

  void Wait(Mutex &mutex) {
    if (::pthread_cond_wait(&this->cond_, mutex.GetNativeHandle()) != 0) {
      ::mysya::util::ThrowSystemErrorException(
          "ConditionVariable::Wait() failed in pthread_cond_init, strerror(%s).",
          ::strerror(errno));
    }
  }

  void NotifyOne() {
    ::pthread_cond_signal(&this->cond_);
  }

  void NotifyAll() {
    ::pthread_cond_broadcast(&this->cond_);
  }

  ::pthread_cond_t *GetNativeHandle() {
    return &this->cond_;
  }

 private:
  ::pthread_cond_t cond_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(ConditionVariable);
};

}  // namespace ioevent
}  // namespace mysya

#endif  // MYSYA_IOEVENT_CONDITION_VARIABLE_H
