#ifndef MYSYA_QSERVICE_EVENT_LOOP_THREAD_POOL_H
#define MYSYA_QSERVICE_EVENT_LOOP_THREAD_POOL_H

#include <stddef.h>

#include <atomic>
#include <vector>

namespace mysya {
namespace ioevent {

class EventLoop;

}  // namespace ioevent
}  // namespace mysya

namespace mysya {
namespace qservice {

class EventLoopThreadPool {
 public:
  class EventLoopThread;
  typedef std::vector<EventLoopThread *> EventLoopThreadVector;

  EventLoopThreadPool(size_t thread_num);
  ~EventLoopThreadPool();

  ::mysya::ioevent::EventLoop *Allocate();
  const EventLoopThreadVector &GetThreads() const;
  void Quit();

 private:
  bool quit_;

  EventLoopThreadVector threads_;
  std::atomic<size_t> current_;
};

}  // namespace qservice
}  // namespace mysya

#endif  // MYSYA_QSERVICE_EVENT_LOOP_THREAD_POOL_HA
