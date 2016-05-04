#include "tutorial/orcas/gateway/server/actor.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

Actor::Actor(int sockfd, ::mysya::ioevent::TcpSocketApp *tcp_socket_app,
    AppServer *app_server)
  : sockfd_(sockfd), tcp_socket_app_(tcp_socket_app),
    host_(app_server) {}

Actor::~Actor() {}

int Actor::GetSockfd() const {
  return this->sockfd_;
}

::mysya::ioevent::TcpSocketApp *Actor::GetTcpSocketApp() {
  return this->tcp_socket_app_;
}

AppServer *Actor::GetHost() {
  return this->host_;
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
