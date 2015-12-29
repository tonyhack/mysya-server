#include <mysya/ioevent/event_channel.h>

#include <fcntl.h>
#include <unistd.h>

#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/event_loop.h>

namespace mysya {
namespace ioevent {

EventChannel::EventChannel()
  : fd_(-1), event_loop_(NULL), attach_id_(0), app_handle_(NULL) {}
EventChannel::EventChannel(void *handle)
  : fd_(-1), event_loop_(NULL), attach_id_(0), app_handle_(handle) {}
EventChannel::~EventChannel() {}

bool EventChannel::SetNonblock() {
  int flags = ::fcntl(this->fd_, F_GETFL, 0);
  if (::fcntl(this->fd_, F_SETFL, flags | O_NONBLOCK) != 0) {
    MYSYA_ERROR("::fcntl O_NONBLOCK failed.");
    return false;
  }

  return true;
}

bool EventChannel::SetCloseExec() {
  int flags = ::fcntl(this->fd_, F_GETFD, 0);
  if (::fcntl(this->fd_, F_SETFD, flags | FD_CLOEXEC) != 0) {
    MYSYA_ERROR("::fcntl FD_CLOEXEC failed.");
    return false;
  }

  return true;
}

bool EventChannel::AttachEventLoop(EventLoop *event_loop) {
  if (this->event_loop_ != NULL) {
    this->DetachEventLoop();
  }

  if (event_loop->AddEventChannel(this) == false) {
    MYSYA_ERROR("EventLoop::AddEventChannel() failed.");
    return false;
  }

  this->event_loop_ = event_loop;
  this->attach_id_ = this->event_loop_->AllocateAttachID();

  return true;
}

void EventChannel::DetachEventLoop() {
  if (this->event_loop_ == NULL) {
    return;
  }

  this->event_loop_->RemoveEventChannel(this);
  this->event_loop_ = NULL;
}

void EventChannel::SetReadCallback(const ReadCallback &cb) {
  this->read_cb_ = cb;
  this->UpdateEventLoop();
}

void EventChannel::ResetReadCallback() {
  ReadCallback cb;
  this->read_cb_.swap(cb);
  this->UpdateEventLoop();
}

void EventChannel::SetWriteCallback(const WriteCallback &cb) {
  this->write_cb_ = cb;
  this->UpdateEventLoop();
}

void EventChannel::ResetWriteCallback() {
  WriteCallback cb;
  this->write_cb_.swap(cb);
  this->UpdateEventLoop();
}

void EventChannel::SetErrorCallback(const ErrorCallback &cb) {
  this->error_cb_ = cb;
  this->UpdateEventLoop();
}

void EventChannel::ResetErrorCallback() {
  ErrorCallback cb;
  this->error_cb_.swap(cb);
  this->UpdateEventLoop();
}

void EventChannel::UpdateEventLoop() {
  if (this->event_loop_ != NULL) {
    this->event_loop_->UpdateEventChannel(this);
  }
}

}  // namespace ioevent
}  // namespace mysya
