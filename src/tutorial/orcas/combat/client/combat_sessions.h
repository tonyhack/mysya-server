#ifndef TUTORIAL_ORCAS_COMBAT_CLIENT_COMBAT_SESSIONS_H
#define TUTORIAL_ORCAS_COMBAT_CLIENT_COMBAT_SESSIONS_H

#include <map>

#include <mysya/codec/protobuf_codec.h>
#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/message_dispatcher.h"

namespace mysya {
namespace ioevent {

class DynamicBuffer;
class SocketAddress;
class TcpSocketApp;

}  // namespace ioevent
}  // namespace mysya

namespace tutorial {
namespace orcas {
namespace combat {
namespace client {

class CombatSession;

class CombatSessions {
 public:
  // <server_id, sockfd>
  typedef std::map<int32_t, int> ConnectionMap;
  // <sockfd, CombatSession *>
  typedef std::map<int, CombatSession *> SessionMap;

  explicit CombatSessions(::mysya::ioevent::TcpSocketApp *tcp_socket_app);
  ~CombatSessions();

  bool AsyncConnect(int32_t server_id,
      const ::mysya::ioevent::SocketAddress &addr, int timeout_ms);

  CombatSession *GetSessionByServerId(int32_t server_id);
  ::mysya::codec::ProtobufCodec *GetCodec();

 private:
  void AddConnection(int32_t server_id, int sockfd);
  int GetConnection(int32_t server_id);
  void RemoveConnection(int32_t server_id);

  void AddSession(int sockfd, CombatSession *session);
  CombatSession *GetSession(int sockfd);
  void RemoveSession(int sockfd);

  void OnConnected(::mysya::ioevent::TcpSocketApp *tcp_socket_app, int sockfd);
  void OnConnectError(::mysya::ioevent::TcpSocketApp *tcp_socket_app, int sockfd, int sys_errno);

  void OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd,
      ::mysya::ioevent::DynamicBuffer *buffer);
  void OnSendCompleted(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code);

  void OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *app,
        const ::google::protobuf::Message *message);

  ::mysya::ioevent::TcpSocketApp *tcp_socket_app_;

  ::mysya::codec::ProtobufCodec codec_;

  ConnectionMap connections_;
  SessionMap sessions_;

  MessageDispatcher message_dispatcher_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatSessions);
};

}  // namespace client
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_CLIENT_COMBAT_SESSIONS_H
