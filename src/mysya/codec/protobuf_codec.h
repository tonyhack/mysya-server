#ifndef MYSYA_PROTOBUF_CODEC_CODEC_H
#define MYSYA_PROTOBUF_CODEC_CODEC_H

#include <functional>

#include <google/protobuf/message.h>

namespace mysya {
namespace ioevent {

class DynamicBuffer;
class TcpSocketApp;

}  // namespace ioevent

namespace codec {

class ProtobufCodec {
 public:
  typedef std::function<void (int, ::mysya::ioevent::TcpSocketApp *,
      const ::google::protobuf::Message *)> MessageCallback;

  ProtobufCodec(::mysya::ioevent::TcpSocketApp *socket_app);
  ~ProtobufCodec();

  void SetMessageCallback(const MessageCallback &cb);
  void ResetMessageCallback();

  int OnMessage(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer);
  int SendMessage(int sockfd, const ::google::protobuf::Message &message);

 private:
  static const int kTypeBytes = 4;
  static const int kHeaderBytes = 4;

  ::mysya::ioevent::TcpSocketApp *socket_app_;

  MessageCallback message_cb_;
};

}  // namespace codec
}  // namespace mysya

#endif  // MYSYA_PROTOBUF_CODEC_CODEC_H
