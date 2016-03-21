#include <mysya/ioevent/event_loop.h>

#include <stdint.h>
#include <string.h>

#include <memory>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include <mysya/ioevent/event_channel.h>
#include <mysya/ioevent/logger.h>
#include <mysya/util/exception.h>

namespace mysya {
namespace ioevent {

#ifdef EPOLLRDHUP
const int EventLoop::kReadEventMask = EPOLLIN | EPOLLPRI | EPOLLRDHUP;
#else
const int EventLoop::kReadEventMask = EPOLLIN | EPOLLPRI;
#endif

const int EventLoop::kWriteEventMask = EPOLLOUT;
const int EventLoop::kErrorEventMask = EPOLLERR | EPOLLHUP;

class EventLoop::AttachIdAllocator {
 public:
  AttachIdAllocator(EventLoop *event_loop)
    : event_loop_(event_loop), id_(0) {}
  ~AttachIdAllocator() {}

  uint64_t Allocate() {
    const ::mysya::util::Timestamp &timestamp = this->event_loop_->GetTimestamp();
    return ((uint64_t)timestamp.GetSecond() << 32) + this->id_++;
  }

 private:
  EventLoop *event_loop_;
  uint32_t id_;
};

EventLoop::EventLoop()
  : quit_(false), epoll_fd_(-1), active_events_(32),
    timing_wheel_(NULL), wakeup_fd_(-1), wakeup_event_channel_(NULL)  {
  this->epoll_fd_ = epoll_create(10240);

  if (-1 == this->epoll_fd_) {
    ::mysya::util::ThrowSystemErrorException(
        "EventLoop::EventLoop(): create event loop failed in epoll_create, strerror(%s).",
        ::strerror(errno));
  }

  int flags = ::fcntl(this->epoll_fd_, F_GETFD, 0);
  if (::fcntl(this->epoll_fd_, F_SETFD, flags | FD_CLOEXEC) != 0) {
    ::mysya::util::ThrowSystemErrorException(
        "EventLoop::EventLoop(): create event loop failed in fcntl, strerror(%s).",
        ::strerror(errno));
  }

  this->timestamp_.SetNow();

  std::unique_ptr<AttachIdAllocator> attach_ids(new (std::nothrow) AttachIdAllocator(this));
  if (attach_ids.get() == NULL) {
    ::mysya::util::ThrowSystemErrorException(
        "EventLoop::EventLoop(): create event loop failed in allocate AttachIdAllocator.");
  }

  std::unique_ptr<TimingWheel> timing_wheel(new (std::nothrow) TimingWheel(10, this));
  if (timing_wheel.get() == NULL) {
    ::mysya::util::ThrowSystemErrorException(
        "EventLoop::EventLoop(): create event loop failed in allocate TimingWheel.");
  }

  std::unique_ptr<EventChannel> wakeup_event_chanel(new (std::nothrow) EventChannel());
  if (wakeup_event_chanel.get() == NULL) {
    ::mysya::util::ThrowSystemErrorException(
        "EventLoop::EventLoop(): create event loop failed in allocate EventChannel.");
  }

  this->attach_ids_ = attach_ids.get();

  this->timing_wheel_ = timing_wheel.get();
  this->timing_wheel_->SetTimestamp(this->timestamp_);

  this->wakeup_event_channel_ = wakeup_event_chanel.get();
  this->wakeup_event_channel_->SetFileDescriptor(this->wakeup_fd_);
  this->wakeup_event_channel_->SetReadCallback(
      std::bind(&EventLoop::OnWakeupRead, this, std::placeholders::_1));
  this->wakeup_event_channel_->AttachEventLoop(this);

  attach_ids.release();
  timing_wheel.release();
  wakeup_event_chanel.release();
}

EventLoop::~EventLoop() {
  this->wakeup_event_channel_->DetachEventLoop();
  this->wakeup_event_channel_->ResetReadCallback();
  delete this->wakeup_event_channel_;
  if (this->wakeup_fd_ != -1) {
    ::close(this->wakeup_fd_);
  }

  delete this->timing_wheel_;
  delete this->attach_ids_;

  if (this->epoll_fd_ != -1) {
    ::close(this->epoll_fd_);
  }
}

bool EventLoop::Looping() const {
  return this->quit_ == false;
}

void EventLoop::Loop() {
  int event_count = 0;
  this->quit_ = false;

  ::mysya::util::Timestamp timestamp = this->timestamp_;
  this->timestamp_.SetNow();

  while (this->quit_ == false) {
    event_count = ::epoll_wait(this->epoll_fd_, &this->active_events_[0],
        this->active_events_.size(), -1);
    if (event_count == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        MYSYA_ERROR("epoll_wait() failed, error(%s)", ::strerror(errno));
        break;
      }
    }

    this->timestamp_.SetNow();

    // event callback.
    for (int i = 0; i < event_count; ++i) {
      struct epoll_event *event = &this->active_events_[i];
      EventChannel *event_channel = static_cast<EventChannel *>(event->data.ptr);

      if (event->events & EventLoop::kReadEventMask) {
        if (this->CheckEventChannelRemoved(event_channel) == true) {
          continue;
        }
        event_channel->GetReadCallback()(event_channel);
      }

      if (event->events & EventLoop::kWriteEventMask) {
        if (this->CheckEventChannelRemoved(event_channel) == true) {
          continue;
        }
        event_channel->GetWriteCallback()(event_channel);
      }

      if (event->events & EventLoop::kErrorEventMask) {
        if (this->CheckEventChannelRemoved(event_channel) == true) {
          continue;
        }
        if (event_channel->GetErrorCallback()) {
          event_channel->GetErrorCallback()(event_channel);
        }
      }
    }

    this->removed_event_channels_.clear();

    this->DoWakeupCallback();
  }
}

void EventLoop::Quit() {
  this->quit_ = true;
}

// TODO
void EventLoop::Wakeup() {
  eventfd_t data = 1;
  ssize_t ret = ::write(this->wakeup_fd_, &data, sizeof(data));
  if (ret == -1 && errno != EAGAIN) {
    MYSYA_ERROR("::write(%d) failed, strerror(%s).",
        this->wakeup_fd_, ::strerror(errno));
  }
}

bool EventLoop::AddEventChannel(EventChannel *channel) {
  struct epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = 0;
  event.data.ptr = channel;

  if (channel->GetReadCallback()) {
    event.events |= EPOLLIN | EPOLLPRI | EPOLLET;
  }
  if (channel->GetWriteCallback()) {
    event.events |= EPOLLOUT | EPOLLET;
  }

  if (::epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD,
        channel->GetFileDescriptor(), &event) != 0) {
    MYSYA_ERROR("epoll_ctl() failed, EPOLL_CTL_ADD fd(%d) error(%s)",
        channel->GetFileDescriptor(), ::strerror(errno));
    return false;
  }

  return true;
}

bool EventLoop::RemoveEventChannel(EventChannel *channel) {
  struct epoll_event event;
  ::memset(&event, 0, sizeof(event));

  if (::epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL,
        channel->GetFileDescriptor(), &event) != 0) {
    MYSYA_ERROR("epoll_ctl*( failed, EPOLL_CTL_DEL fd(%d) errno(%s)",
        channel->GetFileDescriptor(), ::strerror(errno));
    return false;
  }

  // removed event channels.
  this->removed_event_channels_.insert(channel->GetAttachID());

  return true;
}

bool EventLoop::UpdateEventChannel(EventChannel *channel) {
  struct epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = 0;
  event.data.ptr = channel;

  if (channel->GetReadCallback()) {
    event.events |= EPOLLIN | EPOLLPRI | EPOLLET;
  }
  if (channel->GetWriteCallback()) {
    event.events |= EPOLLOUT | EPOLLET;
  }

  if (::epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD,
        channel->GetFileDescriptor(), &event) != 0) {
    MYSYA_ERROR("epoll_ctl() failed, EPOLL_CTL_MOD fd(%d) errno(%s)",
        channel->GetFileDescriptor(), ::strerror(errno));
    return false;
  }

  return true;
}

uint64_t EventLoop::AllocateAttachID() {
  return this->attach_ids_->Allocate();
}

int64_t EventLoop::StartTimer(int expire_ms, const ExpireCallback &cb,
    int call_times) {
  return this->timing_wheel_->AddTimer(this->timestamp_,
      expire_ms, cb, call_times);
}

void EventLoop::StopTimer(int64_t timer_id) {
  this->timing_wheel_->RemoveTimer(timer_id);
}

void EventLoop::PushWakeupCallback(const WakeupCallback &cb) {
  do {
    LockGuard lock(this->wakeup_mutex_);
    this->wakeup_cbs_.push_back(cb);
  } while (false);

  this->Wakeup();
}

#ifndef _MYSYA_DEBUG_
int64_t EventLoop::GetTimerDebugTickCounts() const {
  return this->timing_wheel_->GetDebugTickCounts();
}

void EventLoop::SetTimerDebugTickCounts(int64_t value) const {
  this->timing_wheel_->SetDebugTickCounts(value);
}
#endif  // _MYSYA_DEBUG_

// Event channel should not be use if it has been removed from epoll.
//   See in member function EventLoop::RemoveEventChannel and EventLoop::Loop.
bool EventLoop::CheckEventChannelRemoved(EventChannel *channel) const {
  return this->removed_event_channels_.find(channel->GetAttachID()) !=
    this->removed_event_channels_.end();
}

void EventLoop::OnWakeupRead(EventChannel *event_channel) {
  eventfd_t data = 1;
  ssize_t ret = ::read(this->wakeup_fd_, &data, sizeof(data));
  if (ret == -1 && errno != EAGAIN) {
    MYSYA_ERROR("::read(%d) failed, strerror(%s).",
        this->wakeup_fd_, ::strerror(errno));
  }
}

void EventLoop::DoWakeupCallback() {
  WakeupCallbackVector cbs;

  do {
    LockGuard lock(this->wakeup_mutex_);
    cbs.swap(this->wakeup_cbs_);
  } while (false);

  for (WakeupCallbackVector::iterator iter = cbs.begin();
      iter != cbs.end(); ++iter) {
    (*iter)();
  }
}

}  // namespace ioevent
}  // namespace mysya
