#include "tutorial/orcas/combat/transport_channel.h"

#include <mysya/ioevent/tcp_socket_app.h>

namespace tutorial {
namespace orcas {
namespace combat {

TransportChannel::TransportChannel(int sockfd,
    ::mysya::ioevent::TcpSocketApp *app)
  : sockfd_(sockfd), tcp_socket_app_(app) {}
TransportChannel::~TransportChannel() {}

int TransportChannel::GetSockfd() const {
  return this->sockfd_;
}

::mysya::ioevent::TcpSocketApp *TransportChannel::GetTcpSocketApp() {
  return this->tcp_socket_app_;
}

}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
