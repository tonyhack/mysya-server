#ifndef MASYA_MUTEX_H
#define MASYA_MUTEX_H

#include <pthread.h>

#include <masya/class_util.h>
#include <masya/exception.h>

namespace masya {

class Mutex {
 public:
  Mutex() {
    if (::pthread_mutex_init(&this->mutex_, NULL) != 0) {
      throw SystemErrorException(
          "Mutex::Mutex() failed in pthread_mutex_init.");
    }
  }

  ~Mutex() {
    ::pthread_mutex_destroy(&this->mutex_);
  }

  void Lock() {
    if (::pthread_mutex_lock(&this->mutex_) != 0) {
      throw SystemErrorException(
          "Mutex::Lock() failed in pthread_mutex_lock.");
    }
  }

  void Unlock() {
    if (::pthread_mutex_unlock(&this->mutex_) != 0) {
      throw SystemErrorException(
          "Mutex::Unlock() failed in pthread_mutex_lock.");
    }
  }

  ::pthread_mutex_t *GetNativeHandle() {
    return &this->mutex_;
  }

 private:
  ::pthread_mutex_t mutex_;

  MASYA_DISALLOW_COPY_AND_ASSIGN(Mutex);
};

template <class MutexType>
class LockGuardTempalte {
 public:
  explicit LockGuardTempalte(MutexType &mutex)
    : mutex_(mutex) {
    this->mutex_.Lock();
  }

  ~LockGuardTempalte() {
    this->mutex_.Unlock();
  }

 private:
  MutexType &mutex_;

  MASYA_DISALLOW_COPY_AND_ASSIGN(LockGuardTempalte<MutexType>);
};

typedef LockGuardTempalte<Mutex> LockGuard;

}  // namespace masya

#endif  // MASYA_MUTEX_H
