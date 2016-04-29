#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_APP_SESSION_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_APP_SESSION_H

#include <unordered_set>

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
namespace server {

class AppServer;
class CombatField;
class CombatRoleField;

class AppSession : public ::tutorial::orcas::combat::TransportChannel {
 public:
  typedef std::unordered_set<CombatField *> CombatFieldHashset;
  typedef std::unordered_set<CombatRoleField *> CombatRoleFieldHashset;

  explicit AppSession(int sockfd,
      ::mysya::ioevent::TcpSocketApp *tcp_socket_app, AppServer *host);
  virtual ~AppSession();

  int SendMessage(const ::google::protobuf::Message &message);

  void Add(CombatField *combat);
  void Remove(CombatField *combat);

  void Add(CombatRoleField *role);
  void Remove(CombatRoleField *role);

 private:
  AppServer *host_;

  CombatFieldHashset connected_combats_;
  CombatRoleFieldHashset connected_roles_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(AppSession);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_APP_SESSION_H
