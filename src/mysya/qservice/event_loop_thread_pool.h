#ifndef MYSYA_QSERVICE_EVENT_LOOP_THREAD_POOL_H
#define MYSYA_QSERVICE_EVENT_LOOP_THREAD_POOL_H

#include <stddef.h>

#include <atomic>
#include <vector>

#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/thread.h>

namespace mysya {
namespace ioevent {

class EventLoop;

}  // namespace ioevent
}  // namespace mysya

namespace mysya {
namespace qservice {

class EventLoopThread {
 public:
  EventLoopThread() {}
  ~EventLoopThread() {}

  ::mysya::ioevent::EventLoop event_loop_;
  ::mysya::ioevent::Thread thread_;
};

class EventLoopThreadPool {
 public:
  typedef std::vector<EventLoopThread *> EventLoopThreadVector;

  EventLoopThreadPool(size_t thread_num);
  ~EventLoopThreadPool();

  ::mysya::ioevent::EventLoop *Allocate();
  EventLoopThreadVector &GetThreads();
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
