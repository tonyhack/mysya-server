#ifndef MYSYA_CONDITION_VARIABLE_H
#define MYSYA_CONDITION_VARIABLE_H

#include <pthread.h>
#include <string.h>

#include <mysya/exception.h>
#include <mysya/mutex.h>

namespace mysya {

class ConditionVariable {
 public:
   ConditionVariable() {
     if (::pthread_cond_init(&this->cond_, NULL) != 0) {
       ThrowSystemErrorException(
           "ConditionVariable::ConditionVariable() failed in pthread_cond_init, strerror(%s).",
          ::strerror(errno));
     }
  }
  ~ConditionVariable() {
    ::pthread_cond_destroy(&this->cond_);
  }

  void Wait(Mutex &mutex) {
    if (::pthread_cond_wait(&this->cond_, mutex.GetNativeHandle()) != 0) {
      ThrowSystemErrorException(
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

}  // namespace mysya

#endif  // MYSYA_CONDITION_VARIABLE_H
