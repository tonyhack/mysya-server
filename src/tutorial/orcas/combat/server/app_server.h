#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_APP_SERVER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_APP_SERVER_H

#include <unordered_map>

#include <mysya/ioevent/tcp_socket_app.h>
#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/message_dispatcher.h"
#include "tutorial/orcas/combat/server/combat_message_handler.h"

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace mysya {
namespace codec {

class ProtobufCodec;

}  // namespace codec

namespace ioevent {

class DynamicBuffer;
class EventLoop;
class SocketAddress;

}  // namespace mysya
} // namespace ioevent

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppSession;

class AppServer {
 public:
  typedef std::unordered_map<const ::mysya::ioevent::TcpSocketApp *,
          ::mysya::codec::ProtobufCodec *> CodecHashmap;
  typedef std::unordered_map<int, AppSession *> AppSessionHashMap;

  explicit AppServer(::mysya::ioevent::EventLoop *event_loop,
      int listen_backlog = 128);
  ~AppServer();

  bool Listen(const ::mysya::ioevent::SocketAddress &addr);
  ::mysya::codec::ProtobufCodec *GetProtobufCodec(
      const ::mysya::ioevent::TcpSocketApp *app);
  AppSession *GetSession(int sockfd);
  MessageDispatcher *GetMessageDispatcher();

 private:
  AppSession *RemoveSession(int sockfd);
  bool AddSession(AppSession *session);

  void OnConnected(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd,
      ::mysya::ioevent::DynamicBuffer *buffer);
  void OnSendCompleted(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code);

  void OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *app,
      const ::google::protobuf::Message *message);

  int listen_backlog_;

  ::mysya::ioevent::EventLoop *event_loop_;
  ::mysya::ioevent::TcpSocketApp tcp_socket_app_;

  CodecHashmap protobuf_codecs_;
  AppSessionHashMap app_sessions_;

  MessageDispatcher message_dispatcher_;
  CombatMessageHandler combat_message_handler_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(AppServer);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_APP_SERVER_H
