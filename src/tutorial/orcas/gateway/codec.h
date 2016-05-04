#ifndef TUTORIAL_ORCAS_GATEWAY_CODEC_H
#define TUTORIAL_ORCAS_GATEWAY_CODEC_H

#include <functional>

#include <google/protobuf/message.h>
#include <mysya/util/class_util.h>

#include "tutorial/orcas/gateway/aes.h"

namespace mysya {
namespace ioevent {

class DynamicBuffer;
class TcpSocketApp;

}  // namespace mysya
}  // namespace ioevent

namespace tutorial {
namespace orcas {
namespace gateway {

class Codec {
 public:
  typedef std::function<void (int, ::mysya::ioevent::TcpSocketApp *,
      int, const char *data, int size)> MessageCallback;

  Codec(::mysya::ioevent::TcpSocketApp *socket_app);
  ~Codec();

  void SetMessageCallback(const MessageCallback &cb);
  void ResetMessageCallback();

  int OnMessage(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer);
  int SendMessage(int sockfd, int message_type, const char *data, int size);

 private:
  static unsigned char kAesKey_[16];
  static const int kMaxMessageSize_ = 65535;

  char message_send_buffer_[kMaxMessageSize_];

  Aes aes_;
  ::mysya::ioevent::TcpSocketApp *socket_app_;
  MessageCallback message_cb_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Codec);
};

}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_CODEC_H
