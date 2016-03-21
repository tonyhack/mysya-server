#include <mysya/qservice/message_queue.h>

namespace mysya {
namespace qservice {

struct MessageQueueElement {
  int host_;
  size_t size_;
  char data_[0];
};

MessageQueue::MessageQueue(size_t init_size, size_t ext_size)
  : buffer1_(init_size, ext_size),
    buffer2_(init_size, ext_size),
    consumer_(&buffer1_),
    producer_(&buffer2_) {}

MessageQueue::~MessageQueue() {}

int MessageQueue::Read(int &host, char *data, int size) {
  if (this->consumer_->ReadableBytes() <= 0) {
    return 0;
  }

  const MessageQueueElement *element =
    (const MessageQueueElement *)this->consumer_->ReadBegin();
  if (element->size_ > (size_t)size) {
    return -1;
  }

  int ret_size = element->size_;
  host = element->host_;
  memcpy(data, element->data_, ret_size);
  this->consumer_->ReadBytes(ret_size + sizeof(MessageQueueElement));

  return ret_size;
}

int MessageQueue::Read(int &host, ::mysya::ioevent::DynamicBuffer &buffer) {
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

int MessageQueue::Write(int host, const char *data, int size) {
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

}  // namespace qservice
}  // namespace mysya
