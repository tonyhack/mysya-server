#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_APP_SERVER_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_APP_SERVER_H

#include <unordered_map>

#include <mysya/ioevent/tcp_socket_app.h>

#include "tutorial/orcas/combat/client/combat_sessions.h"
#include "tutorial/orcas/gateway/codec.h"
#include "tutorial/orcas/gateway/server/message_dispatcher.h"

namespace mysya {
namespace ioevent {

class DynamicBuffer;
class EventLoop;
class SocketAddress;

}  // namespace ioevent
}  // namespace mysya

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

class Actor;

class AppServer {
 public:
  typedef std::unordered_map<int, Actor *> ActorHashmap;

  AppServer(::mysya::ioevent::EventLoop *event_loop,
      int listen_backlog = 128);
  ~AppServer();

  bool Listen(const ::mysya::ioevent::SocketAddress &addr);
  int SendMessage(int sockfd, int message_type, ::google::protobuf::Message *message);
  MessageDispatcher *GetMessageDispatcher();

 private:
  void AddActor(Actor *actor);
  Actor *RemoveActor(int sockfd);
  Actor *GetActor(int sockfd);

  void OnConnected(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd,
      ::mysya::ioevent::DynamicBuffer *buffer);
  void OnSendCompleted(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code);

  void OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *socket_app,
      int message_type, const char *data, int size);

  int listen_backlog_;

  ::mysya::ioevent::EventLoop *event_loop_;
  ::mysya::ioevent::TcpSocketApp tcp_socket_app_;
  ::tutorial::orcas::combat::client::CombatSessions combat_clients_;

  Codec codec_;
  MessageDispatcher message_dispatcher_;

  ActorHashmap actors_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(AppServer);
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_APP_SERVER_H
