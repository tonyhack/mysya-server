#include <mysya/codec/protobuf_codec.h>

#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/tcp_socket_app.h>

namespace mysya {
namespace codec {

ProtobufCodec::ProtobufCodec(::mysya::ioevent::TcpSocketApp *socket_app) {}
ProtobufCodec::~ProtobufCodec() {}

void ProtobufCodec::SetMessageCallback(const MessageCallback &cb) {
  this->message_cb_ = cb;
}

void ProtobufCodec::ResetMessageCallback() {
  MessageCallback cb;
  this->message_cb_.swap(cb);
}

static ::google::protobuf::Message *CreateMessage(const std::string &type) {
  ::google::protobuf::Message *message = NULL;

  const ::google::protobuf::Descriptor *descriptor =
    ::google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type);
  if (descriptor != NULL) {
    const ::google::protobuf::Message *prototype =
      ::google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
      if (prototype != NULL) {
        message = prototype->New();
      }
  }

  return message;
}

int ProtobufCodec::OnMessage(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer) {
  int read_message_bytes = 0;

  for (;;) {
    int readable_bytes = buffer->ReadableBytes();
    if (readable_bytes <= 0 || readable_bytes < ProtobufCodec::kHeaderBytes) {
      break;
    }

    char *data = buffer->ReadBegin();

    int32_t message_bytes = *(int32_t *)data;
    if (readable_bytes < message_bytes) {
      break;
    } else if (message_bytes < ProtobufCodec::kTypeBytes) {
      break;
    }

    int16_t type_bytes = *(int16_t *)(data + ProtobufCodec::kHeaderBytes);
    std::string type_name(data + ProtobufCodec::kTypeBytes, type_bytes);

    std::unique_ptr< ::google::protobuf::Message> message(CreateMessage(type_name));
    if (message.get() == NULL) {
      return -1;
    }

    buffer->ReadBytes(message_bytes);
    read_message_bytes += message_bytes;

    if (this->message_cb_) {
      this->message_cb_(sockfd, this->socket_app_, message.get());
    }
  }

  return read_message_bytes;
}

int ProtobufCodec::SendMessage(int sockfd, const ::google::protobuf::Message &message) {
  std::string type_name = message.GetTypeName();

  int32_t type_bytes = type_name.size();
  int32_t message_bytes = message.ByteSize();
  int32_t protocol_bytes = message_bytes + ProtobufCodec::kHeaderBytes +
    ProtobufCodec::kTypeBytes + type_bytes;

  ::mysya::ioevent::DynamicBuffer buffer;
  buffer.Append(&protocol_bytes, ProtobufCodec::kHeaderBytes);
  buffer.Append(&type_bytes, ProtobufCodec::kTypeBytes);
  buffer.Append(type_name.data(), type_name.size());

  buffer.ReserveWritableBytes(protocol_bytes);
  ::google::protobuf::uint8 *begin = (::google::protobuf::uint8 *)buffer.WriteBegin();
  ::google::protobuf::uint8 *end = message.SerializeWithCachedSizesToArray(begin);
  if (end - begin != message_bytes) {
    return -1;
  }

  buffer.WrittenBytes(protocol_bytes);

  if (this->socket_app_->SendMessage(sockfd, buffer.ReadBegin(), buffer.ReadableBytes()) == false) {
    return -1;
  }

  return buffer.ReadableBytes();
}

}  // namespace codec
}  // namespace mysya
