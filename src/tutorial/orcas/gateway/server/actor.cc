#include "tutorial/orcas/gateway/server/actor.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/tcp_socket_app.h>

#include "tutorial/orcas/gateway/server/app_server.h"
#include "tutorial/orcas/gateway/server/combat_actor.h"
#include "tutorial/orcas/gateway/server/combat_actor_manager.h"
#include "tutorial/orcas/gateway/server/combat_manager.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

Actor::Actor(int sockfd, ::mysya::ioevent::TcpSocketApp *tcp_socket_app,
    AppServer *app_server)
  : sockfd_(sockfd), tcp_socket_app_(tcp_socket_app),
    host_(app_server), combat_actor_(NULL) {}

Actor::~Actor() {
  if (this->combat_actor_ != NULL) {
    CombatManager::GetInstance()->Offline(this->combat_actor_);
    this->combat_actor_->SetActor(NULL);
  }
}

int Actor::GetSockfd() const {
  return this->sockfd_;
}

::mysya::ioevent::TcpSocketApp *Actor::GetTcpSocketApp() {
  return this->tcp_socket_app_;
}

AppServer *Actor::GetHost() {
  return this->host_;
}

void Actor::SetCombatActor(CombatActor *combat_actor) {
  this->combat_actor_ = combat_actor;
}

CombatActor *Actor::GetCombatActor() {
  return this->combat_actor_;
}

int Actor::SendMessage(int message_type, const std::string &data) {
  return this->host_->SendMessage(this->sockfd_, message_type, data);
}

int Actor::SendMessage(int message_type, const ::google::protobuf::Message &message) {
  return this->host_->SendMessage(this->sockfd_, message_type, message);
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
