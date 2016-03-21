#include <mysya/qservice/event_loop_thread_pool.h>

#include <functional>
#include <memory>

namespace mysya {
namespace qservice {

EventLoopThreadPool::EventLoopThreadPool(size_t thread_num)
  : quit_(false), current_(0) {
  for (size_t i = 0; i < thread_num; ++i) {
    std::unique_ptr<EventLoopThread> event_loop_thread(new (std::nothrow) EventLoopThread());
    if (event_loop_thread.get() == NULL) {
      ::mysya::util::ThrowSystemErrorException(
          "EventLoopThreadPool::EventLoopThreadPool(): failed in allocate EventLoopThread.");
    }

    event_loop_thread->thread_.Start(
        std::bind(&::mysya::ioevent::EventLoop::Loop, &event_loop_thread->event_loop_));
    this->threads_.push_back(event_loop_thread.get());

    event_loop_thread.release();
  }
}

EventLoopThreadPool::~EventLoopThreadPool() {
  this->Quit();

  for (EventLoopThreadVector::iterator iter = this->threads_.begin();
      iter != this->threads_.end(); ++iter) {
    delete *iter;
  }
}

::mysya::ioevent::EventLoop *EventLoopThreadPool::Allocate() {
  size_t current_pos = this->current_.fetch_add(1);
  return &this->threads_[current_pos % this->threads_.size()]->event_loop_;
}

EventLoopThreadPool::EventLoopThreadVector &EventLoopThreadPool::GetThreads() {
  return this->threads_;
}

const EventLoopThreadPool::EventLoopThreadVector &EventLoopThreadPool::GetThreads() const {
  return this->threads_;
}

void EventLoopThreadPool::Quit() {
  if (this->quit_ == true) {
    return;
  }

  for (EventLoopThreadVector::iterator iter = this->threads_.begin();
      iter != this->threads_.end(); ++iter) {
    EventLoopThread *event_loop_thread = *iter;
    event_loop_thread->event_loop_.Wakeup();
    event_loop_thread->event_loop_.Quit();
    event_loop_thread->thread_.Join();
  }

  this->quit_ = true;
}

}  // namespace qservice
}  // namespace mysya
