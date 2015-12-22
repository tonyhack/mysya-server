#include <mysya/event_loop.h>

#include <stdint.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>

#include <vector>

#include <unistd.h>

#include <mysya/exception.h>
#include <mysya/event_channel.h>
#include <mysya/logger.h>
#include <mysya/timing_wheel.h>

namespace mysya {

#ifdef EPOLLRDHUP
const int EventLoop::kReadEventMask = EPOLLIN | EPOLLPRI | EPOLLRDHUP;
#else
const int EventLoop::kReadEventMask = EPOLLIN | EPOLLPRI;
#endif

const int EventLoop::kWriteEventMask = EPOLLOUT;
const int EventLoop::kErrorEventMask = EPOLLERR | EPOLLHUP;

EventLoop::EventLoop()
  : quit_(false), epoll_fd_(-1), active_events_(32),
    timing_wheel_(NULL) {
  this->epoll_fd_ = epoll_create(10240);

  if (-1 == this->epoll_fd_) {
    throw SystemErrorException(
        "EventLoop::EventLoop(): create event loop failed in epoll_create");
  }

  int flags = ::fcntl(this->epoll_fd_, F_GETFD, 0);
  if (::fcntl(this->epoll_fd_, F_SETFD, flags | FD_CLOEXEC) != 0) {
    throw SystemErrorException(
        "EventLoop::EventLoop(): create event loop failed in fcntl");
  }

  this->timestamp_.SetNow();

  this->timing_wheel_ = new (std::nothrow) TimingWheel(10, this);
  if (this->timing_wheel_ == NULL) {
    throw SystemErrorException(
        "EventLoop::EventLoop(): create event loop failed in allocate TimingWheel.");
  }

  this->timing_wheel_->SetTimestamp(this->timestamp_);
}

EventLoop::~EventLoop() {
  delete this->timing_wheel_;

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

  Timestamp timestamp = this->timestamp_;
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
  }
}

void EventLoop::Quit() {
  this->quit_ = true;
}

// TODO
void EventLoop::Wakeup() {
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
  this->removed_event_channels_.insert((intptr_t)channel);

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

int64_t EventLoop::StartTimer(int expire_ms, const TimerCallback &cb,
    int call_times) {
  return this->timing_wheel_->AddTimer(this->timestamp_,
      expire_ms, cb, call_times);
}

void EventLoop::StopTimer(int64_t timer_id) {
  this->timing_wheel_->RemoveTimer(timer_id);
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
  return this->removed_event_channels_.find((intptr_t)channel) !=
    this->removed_event_channels_.end();
}

}  // namespace mysya
