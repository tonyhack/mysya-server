#include "tutorial/orcas/combat/server/user_combat_message_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/transport_channel.h"
#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_role_field_manager.h"
#include "tutorial/orcas/protocol/cc/message.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

UserCombatMessageHandler::UserCombatMessageHandler(AppServer *host)
  : host_(host) {}
UserCombatMessageHandler::~UserCombatMessageHandler() {}

void UserCombatMessageHandler::SetMessageHandlers() {
  this->host_->GetUserMessageDispatcher()->SetMessageCallback(
      ::protocol::MESSAGE_COMBAT_ACTION_REQUEST,
      std::bind(&UserCombatMessageHandler::OnMessageCombatActionRequest,
        this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3));
}

void UserCombatMessageHandler::ResetMessageHandlers() {
  this->host_->GetUserMessageDispatcher()->ResetMessageCallback(
      ::protocol::MESSAGE_COMBAT_ACTION_REQUEST);
}

void UserCombatMessageHandler::OnMessageCombatActionRequest(
    CombatRoleField *role, const char *data, int size) {
  ::protocol::MessageCombatActionRequest message;
  if (message.ParseFromArray(data, size) == false) {
    MYSYA_ERROR("MessageCombatActionRequest::ParseFromArray() failed.");
    return;
  }

  role->DoAction(message.action());
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
