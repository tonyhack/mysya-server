#include <mysya/qservice/message_queue.h>

#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/logger.h>

namespace mysya {
namespace qservice {

struct MessageQueueElement {
  int host_;
  size_t size_;
  char data_[0];
};

MessageQueue::MessageQueue(::mysya::ioevent::EventLoop *producer_loop,
    ::mysya::ioevent::EventLoop *consumer_loop, const ReadCallback &cb,
    int max_expoired_msec, size_t init_size, size_t ext_size)
  : producer_loop_(producer_loop),
    consumer_loop_(consumer_loop),
    buffer1_(init_size, ext_size),
    buffer2_(init_size, ext_size),
    producer_(&buffer2_),
    consumer_(&buffer1_),
    read_cb_(cb),
    max_pending_read_expired_mesec_(max_expoired_msec),
    pending_read_expired_msec_(max_expoired_msec),
    pending_read_timer_id_(-1),
    pending_read_num_(0) {
  this->SetPendingReadTimer();
}

MessageQueue::~MessageQueue() {
  this->RemovePendingReadTimer();
}

int MessageQueue::Push(int host, const char *data, int size) {
  this->pending_read_num_.fetch_add(1);

  ::mysya::ioevent::LockGuard lock_write(this->write_mutex_);

  size_t write_size = size + sizeof(MessageQueueElement);
  this->producer_->ReserveWritableBytes(write_size);

  MessageQueueElement *element = (MessageQueueElement *)this->producer_->WriteBegin();
  element->host_ = host;
  element->size_ = size;
  memcpy(element->data_, data, size);

  this->producer_->WrittenBytes(write_size);

  return size;
}

void MessageQueue::Exchange() {
  ::mysya::ioevent::DynamicBuffer *swap = this->consumer_;
  this->consumer_ = this->producer_;
  this->producer_ = swap;
}

int MessageQueue::Pop(int &host, ::mysya::ioevent::DynamicBuffer &buffer) {
  if (this->consumer_->ReadableBytes() <= 0) {
    return 0;
  }

  const MessageQueueElement *element =
    (const MessageQueueElement *)this->consumer_->ReadBegin();

  int ret_size = element->size_;
  host = element->host_;
  buffer.ReserveWritableBytes(ret_size);
  buffer.Append(element->data_, element->size_);
  this->consumer_->ReadBytes(ret_size + sizeof(MessageQueueElement));

  return ret_size;
}

void MessageQueue::SetPendingReadTimer() {
  ::mysya::util::Timestamp now_timestamp = this->consumer_loop_->GetTimestamp();
  int pending_num = this->pending_read_num_.load();
  int64_t distance_msec = now_timestamp.DistanceMillisecond(this->read_timestamp_);

  if (pending_num <= 0) {
    this->pending_read_expired_msec_ = this->pending_read_expired_msec_ * 2 + 1;
  } else {
    this->pending_read_expired_msec_ = distance_msec / pending_num + 1;
  }

  if (this->pending_read_expired_msec_ > this->max_pending_read_expired_mesec_) {
    this->pending_read_expired_msec_ = this->max_pending_read_expired_mesec_;
  }

  this->pending_read_timer_id_ = this->consumer_loop_->StartTimer(this->pending_read_expired_msec_,
      std::bind(&MessageQueue::OnExporedPendingRead, this, std::placeholders::_1), 1);
}

void MessageQueue::RemovePendingReadTimer() {
  if (this->pending_read_timer_id_ != -1) {
    this->consumer_loop_->StopTimer(this->pending_read_timer_id_);
  }
}

void MessageQueue::OnExporedPendingRead(int64_t timer_id) {
  this->SetPendingReadTimer();
  this->pending_read_num_.store(0);
  this->read_timestamp_ = this->consumer_loop_->GetTimestamp();

  int host = -1;

  ::mysya::ioevent::LockGuard lock_read(this->read_mutex_);
  for (;;) {
    int read_size = this->Pop(host, this->read_buffer_);
    if (read_size <= 0) {
      break;
    }

    if (this->read_cb_) {
      read_cb_(host, this->read_buffer_.ReadBegin(), read_size);
    }

    this->read_buffer_.ReadBytes(read_size);
  }

  ::mysya::ioevent::LockGuard lock_write(this->write_mutex_);
  this->Exchange();
}

}  // namespace qservice
}  // namespace mysya
