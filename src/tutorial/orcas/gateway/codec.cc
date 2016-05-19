#include "tutorial/orcas/gateway/codec.h"

#include <string>
#include <google/protobuf/message.h>

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/tcp_socket_app.h>

namespace tutorial {
namespace orcas {
namespace gateway {

#pragma pack(push, 1)

struct ProtocolHeader {
  int32_t type_;
  int64_t account_;
};

#pragma pack(pop)

unsigned char Codec::kAesKey_[16] = {
  2, 3, 4, 5, 2, 4, 1, 8, 5, 98, 5, 3, 64, 3, 2, 0,
};

Codec::Codec(::mysya::ioevent::TcpSocketApp *socket_app)
  : aes_(kAesKey_), socket_app_(socket_app) {}
Codec::~Codec() {}

void Codec::SetMessageCallback(const MessageCallback &cb) {
  this->message_cb_ = cb;
}

void Codec::ResetMessageCallback() {
  MessageCallback cb;
  this->message_cb_.swap(cb);
}

#define MESSAGE_LENGTH_SIZE sizeof(uint16_t)

int Codec::OnMessage(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer) {
  MYSYA_DEBUG("Codec::OnMessage.");

  const int kProtocoMinSize = MESSAGE_LENGTH_SIZE + sizeof(ProtocolHeader);

  int read_message_bytes = 0;

  for (;;) {
    int readable_bytes = buffer->ReadableBytes() - read_message_bytes;
    if (readable_bytes <= 0 || readable_bytes < kProtocoMinSize) {
      break;
    }

    char *protocol_ptr = buffer->ReadBegin() + read_message_bytes;
    uint16_t message_size = *(uint16_t *)protocol_ptr;
    if (readable_bytes < message_size + (int)MESSAGE_LENGTH_SIZE) {
      break;
    }

    const ProtocolHeader *header =
      (const ProtocolHeader *)(protocol_ptr + MESSAGE_LENGTH_SIZE);

    char *data = protocol_ptr + kProtocoMinSize;
    int data_size = message_size - sizeof(ProtocolHeader);

    this->aes_.Decode(data, data_size);

    if (this->message_cb_) {
      this->message_cb_(sockfd, this->socket_app_, header->type_, data, data_size);
    }

    read_message_bytes += message_size + MESSAGE_LENGTH_SIZE + sizeof(ProtocolHeader);
  }

  buffer->ReadBytes(read_message_bytes);

  return read_message_bytes;
}

int Codec::SendMessage(int sockfd, int message_type, const char *data, int size) {
  const int kProtocoMinSize = MESSAGE_LENGTH_SIZE + sizeof(ProtocolHeader);

  char *data_buffer = this->message_send_buffer_ + kProtocoMinSize;
  int data_buffer_size = kMaxMessageSize_ - kProtocoMinSize;

  if (data_buffer_size < size) {
    return -1;
  }

  *(uint16_t *)this->message_send_buffer_ = size + sizeof(ProtocolHeader);

  ProtocolHeader *header = (ProtocolHeader *)(this->message_send_buffer_ + MESSAGE_LENGTH_SIZE);
  header->type_ = message_type;
  header->account_ = 0;

  memcpy(data_buffer, data, size);
  this->aes_.Encode(data_buffer, size);

  return this->socket_app_->SendMessage(sockfd, this->message_send_buffer_,
      size + kProtocoMinSize);
}

#undef MESSAGE_LENGTH_SIZE

}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
