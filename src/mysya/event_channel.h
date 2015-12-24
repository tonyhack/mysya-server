#ifndef MYSYA_EVENT_CHANNEL_H
#define MYSYA_EVENT_CHANNEL_H

#include <stdint.h>

#include <functional>

#include <mysya/class_util.h>

namespace mysya {

class EventLoop;

class EventChannel {
 public:
  typedef std::function<void (EventChannel *)> ReadCallback;
  typedef std::function<void (EventChannel *)> WriteCallback;
  typedef std::function<void (EventChannel *)> ErrorCallback;

  EventChannel();
  explicit EventChannel(void *handle);
  ~EventChannel();

  bool SetNonblock();
  bool SetCloseExec();

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

  EventLoop *GetEventLoop() const { return this->event_loop_; }

  void SetAttachID(uint64_t value) { this->attach_id_ = value; }
  uint64_t GetAttachID() const { return this->attach_id_; }

  void SetAppHandle(void *handle) { this->app_handle_ = handle; }
  void *GetAppHandle() const { return this->app_handle_; }

 private:
  void UpdateEventLoop();

  int fd_;
  EventLoop *event_loop_;
  uint64_t attach_id_;

  ReadCallback read_cb_;
  WriteCallback write_cb_;
  ErrorCallback error_cb_;

  void *app_handle_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(EventChannel);
};

}  // namespace mysya

#endif  // MYSYA_EVENT_CHANNEL_H
