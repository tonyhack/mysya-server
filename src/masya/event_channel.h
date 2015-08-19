#ifndef MASYA_EVENT_CHANNEL_H
#define MASYA_EVENT_CHANNEL_H

#include <functional>

#include <masya/class_util.h>

namespace masya {

class EventLoop;

class EventChannel {
 public:
  typedef std::function<void ()> ReadCallback;
  typedef std::function<void ()> WriteCallback;
  typedef std::function<void ()> ErrorCallback;

  EventChannel();
  ~EventChannel();

  bool AttachEventLoop(EventLoop *event_loop);
  void DetachEventLoop();

  void SetReadCallback(const ReadCallback &cb);
  void ResetReadCallback();

  void SetWriteCallback(const WriteCallback &cb);
  void ResetWriteCallback();

  void SetErrorCallback(const ErrorCallback &cb);
  void ResetErrorCallback();

  const ReadCallback &GetReadCallback() const { return this->read_cb_; }
  const WriteCallback &GetWriteCallback() const { return this->write_cb_; }
  const ErrorCallback &GetErrorCallback() const { return this->error_cb_; }

  void SetFileDescriptor(int value) { this->fd_ = value; }
  int GetFileDescriptor() const { return this->fd_; }

 private:
  void UpdateEventLoop();

  int fd_;
  EventLoop *event_loop_;

  ReadCallback read_cb_;
  WriteCallback write_cb_;
  ErrorCallback error_cb_;

  MASYA_DISALLOW_COPY_AND_ASSIGN(EventChannel);
};

}  // namespace masya

#endif  // MASYA_EVENT_CHANNEL_H
