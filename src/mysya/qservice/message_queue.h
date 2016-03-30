#ifndef MYSYA_QSERVICE_MESSAGE_QUEUE_H
#define MYSYA_QSERVICE_MESSAGE_QUEUE_H

#include <atomic>
#include <functional>

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/mutex.h>
#include <mysya/util/timestamp.h>

namespace mysya {
namespace ioevent {

class EventLoop;

}  // namespace ioevent
}  // namespace mysya

namespace mysya {
namespace qservice {

class MessageQueue {
 public:
  typedef std::function<void (int, const char *, int)> ReadCallback;

  MessageQueue(::mysya::ioevent::EventLoop *producer_loop,
      ::mysya::ioevent::EventLoop *consumer_loop, const ReadCallback &cb,
      int max_expoired_msec, size_t init_size = 1024, size_t ext_size = 256);
  ~MessageQueue();

  int Push(int host, const char *data, int size);

 private:
  void Exchange();
  int Pop(int &host, ::mysya::ioevent::DynamicBuffer &buffer);

  void SetPendingReadTimer();
  void RemovePendingReadTimer();
  void OnExporedPendingRead(int64_t timer_id);

  ::mysya::ioevent::EventLoop *producer_loop_;
  ::mysya::ioevent::EventLoop *consumer_loop_;

  ::mysya::ioevent::DynamicBuffer buffer1_;
  ::mysya::ioevent::DynamicBuffer buffer2_;
  ::mysya::ioevent::DynamicBuffer *producer_;
  ::mysya::ioevent::DynamicBuffer *consumer_;

  ::mysya::ioevent::Mutex read_mutex_;
  ::mysya::ioevent::Mutex write_mutex_;

  ReadCallback read_cb_;
  ::mysya::ioevent::DynamicBuffer read_buffer_;

  uint32_t max_pending_read_expired_mesec_;
  uint32_t pending_read_expired_msec_;
  int64_t pending_read_timer_id_;
  std::atomic<int> pending_read_num_;
  ::mysya::util::Timestamp read_timestamp_;
};

}  // namespace qservice
}  // namespace mysya

#endif  // MYSYA_QSERVICE_MESSAGE_QUEUE_H
