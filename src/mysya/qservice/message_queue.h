#ifndef MYSYA_QSERVICE_MESSAGE_QUEUE_H
#define MYSYA_QSERVICE_MESSAGE_QUEUE_H

#include <functional>

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/mutex.h>

namespace mysya {
namespace qservice {

class MessageQueue {
 public:
  MessageQueue(size_t init_size = 1024, size_t ext_size = 256);
  ~MessageQueue();

  int Read(int &host, char *data, int size);
  int Read(int &host, ::mysya::ioevent::DynamicBuffer &buffer);
  int Write(int host, const char *data, int size);

  void Exchange();

  ::mysya::ioevent::Mutex &GetReadMutex() { return this->read_mutex_; }
  ::mysya::ioevent::Mutex &GetWriteMutex() { return this->write_mutex_; }

 private:
  ::mysya::ioevent::DynamicBuffer buffer1_;
  ::mysya::ioevent::DynamicBuffer buffer2_;
  ::mysya::ioevent::DynamicBuffer *consumer_;
  ::mysya::ioevent::DynamicBuffer *producer_;

  ::mysya::ioevent::Mutex read_mutex_;
  ::mysya::ioevent::Mutex write_mutex_;
};

}  // namespace qservice
}  // namespace mysya

#endif  // MYSYA_QSERVICE_MESSAGE_QUEUE_H
