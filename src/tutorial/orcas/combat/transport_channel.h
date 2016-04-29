#ifndef TUTORIAL_ORCAS_COMBAT_TRANSPORT_CHANNEL_H
#define TUTORIAL_ORCAS_COMBAT_TRANSPORT_CHANNEL_H

#include <mysya/util/class_util.h>

namespace mysya {
namespace ioevent {

class TcpSocketApp;

}  // namespace ioevent
}  // namespace mysya

namespace tutorial {
namespace orcas {
namespace combat {

class TransportChannel {
 public:
  TransportChannel(int sockfd, ::mysya::ioevent::TcpSocketApp *app);
  virtual ~TransportChannel();

  int GetSockfd() const;
  ::mysya::ioevent::TcpSocketApp *GetTcpSocketApp();

 protected:
  int sockfd_;
  ::mysya::ioevent::TcpSocketApp *tcp_socket_app_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(TransportChannel);
};

}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_TRANSPORT_CHANNEL_H
