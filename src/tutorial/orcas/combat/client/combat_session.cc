#include "tutorial/orcas/combat/client/combat_session.h"

#include <mysya/codec/protobuf_codec.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/client/combat_sessions.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace client {

CombatSession::CombatSession(int sockfd, int32_t server_id,
    ::mysya::ioevent::TcpSocketApp *tcp_socket_app, CombatSessions *host)
  : TransportChannel(sockfd, tcp_socket_app),  server_id_(server_id),
    tcp_socket_app_(tcp_socket_app), host_(host) {}

CombatSession::~CombatSession() {}                                 
                                  
int32_t CombatSession::GetServerId() const {
  return this->server_id_;
}

CombatSessions *CombatSession::GetHost() {
  return this->host_;
}

int CombatSession::SendMessage(const ::google::protobuf::Message &message) {
  ::mysya::codec::ProtobufCodec *codec = this->host_->GetCodec();
  if (codec == NULL) {
    MYSYA_ERROR("[COMBAT_SESSION] CombatSessions:GetProtobufCodec() faield.");
    return -1;
  }

  return codec->SendMessage(this->GetSockfd(), message);
}

}  // namespace client
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
