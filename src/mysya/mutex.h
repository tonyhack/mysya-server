#ifndef MYSYA_MUTEX_H
#define MYSYA_MUTEX_H

#include <pthread.h>
#include <string.h>

#include <mysya/class_util.h>
#include <mysya/exception.h>

namespace mysya {

class Mutex {
 public:
  Mutex() {
    if (::pthread_mutex_init(&this->mutex_, NULL) != 0) {
      ThrowSystemErrorException(
          "Mutex::Mutex() failed in pthread_mutex_init, strerror(%s).",
          ::strerror(errno));
    }
  }

  ~Mutex() {
    ::pthread_mutex_destroy(&this->mutex_);
  }

  void Lock() {
    if (::pthread_mutex_lock(&this->mutex_) != 0) {
      ThrowSystemErrorException(
          "Mutex::Lock() failed in pthread_mutex_lock, strerror(%s).",
          ::strerror(errno));
    }
  }

  void Unlock() {
    if (::pthread_mutex_unlock(&this->mutex_) != 0) {
      ThrowSystemErrorException(
          "Mutex::Unlock() failed in pthread_mutex_lock, strerror(%s).",
          ::strerror(errno));
    }
  }

  ::pthread_mutex_t *GetNativeHandle() {
    return &this->mutex_;
  }

 private:
  ::pthread_mutex_t mutex_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Mutex);
};

template <class MutexType>
class LockGuardTemplate {
 public:
  explicit LockGuardTemplate(MutexType &mutex)
    : mutex_(mutex) {
    this->mutex_.Lock();
  }

  ~LockGuardTemplate() {
    this->mutex_.Unlock();
  }

 private:
  MutexType &mutex_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(LockGuardTemplate<MutexType>);
};

typedef LockGuardTemplate<Mutex> LockGuard;

}  // namespace mysya

#endif  // MYSYA_MUTEX_H
