#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_ACTOR_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_ACTOR_H

#include <string>

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace mysya {
namespace ioevent {

class TcpSocketApp;

}  // namespace ioevent
}  // namespace mysya

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

class AppServer;
class CombatActor;

class Actor {
 public:
  Actor(int sockfd, ::mysya::ioevent::TcpSocketApp *tcp_socket_app,
      AppServer *app_server);
  ~Actor();

  int GetSockfd() const;
  ::mysya::ioevent::TcpSocketApp *GetTcpSocketApp();
  AppServer *GetHost();

  void SetCombatActor(CombatActor *combat_actor);
  CombatActor *GetCombatActor();

  int SendMessage(int message_type, const std::string &data);
  int SendMessage(int message_type, const ::google::protobuf::Message &message);

 private:
  int sockfd_;
  ::mysya::ioevent::TcpSocketApp *tcp_socket_app_;

  AppServer *host_;
  CombatActor *combat_actor_;
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_ACTOR_H
