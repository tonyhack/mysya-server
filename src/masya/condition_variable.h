#ifndef MASYA_CONDITION_VARIABLE_H
#define MASYA_CONDITION_VARIABLE_H

#include <pthread.h>

#include "masya/exception.h"
#include "masya/mutex.h"

namespace masya {

class ConditionVariable {
 public:
   ConditionVariable() {
     if (::pthread_cond_init(&this->cond_, NULL) != 0) {
       throw SystemErrorException(
           "ConditionVariable::ConditionVariable() failed in pthread_cond_init");
     }
  }
  ~ConditionVariable() {
    ::pthread_cond_destroy(&this->cond_);
  }

  void Wait(Mutex &mutex) {
    if (::pthread_cond_wait(&this->cond_, mutex.GetNativeHandle()) != 0) {
      throw SystemErrorException(
          "ConditionVariable::Wait() failed in pthread_cond_init");
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

  MASYA_DISALLOW_COPY_AND_ASSIGN(ConditionVariable);
};

}  // namespace masya

#endif  // MASYA_CONDITION_VARIABLE_H
