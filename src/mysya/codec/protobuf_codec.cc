#include <mysya/codec/protobuf_codec.h>

#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/tcp_socket_app.h>
#include <mysya/util/hex_dump.h>

namespace mysya {
namespace codec {

ProtobufCodec::ProtobufCodec(::mysya::ioevent::TcpSocketApp *socket_app)
  : socket_app_(socket_app) {}
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
    if (readable_bytes <= 0 || readable_bytes < kHeaderBytes) {
      break;
    }

    char *data = buffer->ReadBegin();

    int32_t protocol_bytes = *(int32_t *)data;
    if (readable_bytes < protocol_bytes) {
      break;
    } else if (protocol_bytes < kTypeBytes) {
      break;
    }

    int16_t type_bytes = *(int16_t *)(data + kHeaderBytes);
    std::string type_name(data + kHeaderBytes +
        kTypeBytes, type_bytes);

    std::unique_ptr< ::google::protobuf::Message> message(CreateMessage(type_name));
    if (message.get() == NULL) {
      MYSYA_ERROR("CreateMessage(%s) failed.", type_name.data());
      return -1;
    }

    do {
      if (message->ParseFromArray(data + kHeaderBytes + kTypeBytes + type_bytes,
            protocol_bytes - kHeaderBytes - kTypeBytes - type_bytes) == false) {
        MYSYA_ERROR("%s:ParseFromArray() size(%d) failed.",
            message->GetTypeName().data(), protocol_bytes - kHeaderBytes -
            kTypeBytes - type_bytes);
        break;
      }
      // ::mysya::util::Hexdump(data + kHeaderBytes + kTypeBytes + type_bytes,
      //     protocol_bytes - kHeaderBytes - kTypeBytes - type_bytes,
      //     std::string("receive " + message->GetTypeName()).data());

      if (this->message_cb_) {
        this->message_cb_(sockfd, this->socket_app_, message.get());
      }
    } while (false);

    buffer->ReadBytes(protocol_bytes);
    read_message_bytes += protocol_bytes;
  }

  return read_message_bytes;
}

int ProtobufCodec::SendMessage(int sockfd, const ::google::protobuf::Message &message) {
  std::string type_name = message.GetTypeName();

  int32_t type_bytes = type_name.size();
  int32_t message_bytes = message.ByteSize();
  int32_t protocol_bytes = message_bytes + kHeaderBytes +
    kTypeBytes + type_bytes;

  ::mysya::ioevent::DynamicBuffer buffer;
  buffer.ReserveWritableBytes(protocol_bytes);

  buffer.Append(&protocol_bytes, kHeaderBytes);
  buffer.Append(&type_bytes, kTypeBytes);
  buffer.Append(type_name.data(), type_name.size());

  if (message.SerializeToArray(buffer.WriteBegin(), message_bytes) == false) {
    MYSYA_ERROR("%s.SerializeToArray() failed.", type_name.data());
    return -1;
  }

  // ::mysya::util::Hexdump(buffer.WriteBegin(), message_bytes,
  //     std::string("send " + type_name).data());

  buffer.WrittenBytes(message_bytes);

  if (this->socket_app_->SendMessage(sockfd, buffer.ReadBegin(), buffer.ReadableBytes()) == false) {
    return -1;
  }

  return buffer.ReadableBytes();
}

}  // namespace codec
}  // namespace mysya
