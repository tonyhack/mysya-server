#ifndef MYSYA_IOEVENT_MUTEX_H
#define MYSYA_IOEVENT_MUTEX_H

#include <pthread.h>
#include <string.h>

#include <mysya/util/class_util.h>
#include <mysya/util/exception.h>

namespace mysya {
namespace ioevent {

class Mutex {
 public:
  Mutex() {
    if (::pthread_mutex_init(&this->mutex_, NULL) != 0) {
      ::mysya::util::ThrowSystemErrorException(
          "Mutex::Mutex() failed in pthread_mutex_init, strerror(%s).",
          ::strerror(errno));
    }
  }

  ~Mutex() {
    ::pthread_mutex_destroy(&this->mutex_);
  }

  void Lock() {
    if (::pthread_mutex_lock(&this->mutex_) != 0) {
      ::mysya::util::ThrowSystemErrorException(
          "Mutex::Lock() failed in pthread_mutex_lock, strerror(%s).",
          ::strerror(errno));
    }
  }

  void Unlock() {
    if (::pthread_mutex_unlock(&this->mutex_) != 0) {
      ::mysya::util::ThrowSystemErrorException(
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

}  // namespace ioevent
}  // namespace mysya

#endif  // MYSYA_IOEVENT_MUTEX_H
