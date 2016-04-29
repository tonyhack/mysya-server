#include "tutorial/orcas/combat/server/app_session.h"

#include <mysya/codec/protobuf_codec.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

AppSession::AppSession(int sockfd,
    ::mysya::ioevent::TcpSocketApp *tcp_socket_app, AppServer *host)
  : TransportChannel(sockfd, tcp_socket_app), host_(host) {}

AppSession::~AppSession() {
  for (CombatFieldHashset::iterator iter = this->connected_combats_.begin();
      iter != this->connected_combats_.end(); ++iter) {
    (*iter)->ResetAppSession();
  }

  for (CombatRoleFieldHashset::iterator iter = this->connected_roles_.begin();
      iter != this->connected_roles_.end(); ++iter) {
    (*iter)->ResetAppSession();
  }
}

int AppSession::SendMessage(const ::google::protobuf::Message &message) {
  ::mysya::codec::ProtobufCodec *codec =
    this->host_->GetProtobufCodec(this->GetTcpSocketApp());
  if (codec == NULL) {
    MYSYA_ERROR("[APP_SESSION] AppServer::GetProtobufCodec(%p) failed.",
        this->GetTcpSocketApp());
    return -1;
  }

  return codec->SendMessage(this->GetSockfd(), message);
}

void AppSession::Add(CombatField *combat) {
  this->connected_combats_.insert(combat);
}

void AppSession::Remove(CombatField *combat) {
  this->connected_combats_.erase(combat);
}

void AppSession::Add(CombatRoleField *role) {
  this->connected_roles_.insert(role);
}

void AppSession::Remove(CombatRoleField *role) {
  this->connected_roles_.erase(role);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
