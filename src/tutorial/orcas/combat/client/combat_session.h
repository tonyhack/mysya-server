#ifndef TUTORIAL_ORCAS_COMBAT_CLIENT_COMBAT_SESSION_H
#define TUTORIAL_ORCAS_COMBAT_CLIENT_COMBAT_SESSION_H

#include <stdint.h>

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/transport_channel.h"

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
namespace combat {
namespace client {

class CombatSessions;

class CombatSession : public ::tutorial::orcas::combat::TransportChannel {
 public:
  explicit CombatSession(int sockfd, int32_t server_id,
      ::mysya::ioevent::TcpSocketApp *tcp_socket_app, CombatSessions *host);
  virtual ~CombatSession();

  int32_t GetServerId() const;
  CombatSessions *GetHost();

  int SendMessage(const ::google::protobuf::Message &message);

 private:
  int32_t server_id_;
  ::mysya::ioevent::TcpSocketApp *tcp_socket_app_;
  CombatSessions *host_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatSession);
};

}  // namespace client
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_CLIENT_COMBAT_SESSION_H
