#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_ACTOR_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_ACTOR_H

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

class Actor {
 public:
  Actor(int sockfd, ::mysya::ioevent::TcpSocketApp *tcp_socket_app,
      AppServer *app_server);
  ~Actor();

  int GetSockfd() const;
  ::mysya::ioevent::TcpSocketApp *GetTcpSocketApp();
  AppServer *GetHost();

 private:
  int sockfd_;
  ::mysya::ioevent::TcpSocketApp *tcp_socket_app_;

  AppServer *host_;
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_ACTOR_H
