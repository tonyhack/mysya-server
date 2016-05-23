#include "tutorial/orcas/combat/server/combat_event_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"
#include "tutorial/orcas/protocol/cc/message.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

CombatEventHandler::CombatEventHandler(AppServer *host)
  : host_(host),
    event_token_build_action_(0),
    event_token_move_action_(0) {}
CombatEventHandler::~CombatEventHandler() {}

void CombatEventHandler::SetHandlers() {
  this->event_token_build_action_ =
    this->host_->GetEventDispatcher()->Attach(event::EVENT_COMBAT_BUILD_ACTION,
        std::bind(&CombatEventHandler::EventCombatBuildAction, this, std::placeholders::_1));
  this->event_token_move_action_ =
    this->host_->GetEventDispatcher()->Attach(event::EVENT_COMBAT_MOVE_ACTION,
        std::bind(&CombatEventHandler::EventCombatMoveAction, this, std::placeholders::_1));
}

void CombatEventHandler::ResetHandlers() {
  this->host_->GetEventDispatcher()->Detach(this->event_token_move_action_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_build_action_);
}

void CombatEventHandler::EventCombatBuildAction(const ProtoMessage *data) {
  const event::EventCombatBuildAction *event = (const event::EventCombatBuildAction *)data;

  CombatField *combat_field =
    CombatFieldManager::GetInstance()->Get(event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  combat_field->PushAction(event->action());

  ::protocol::MessageCombatActionSync message;
  *message.mutable_action() = event->action();
  combat_field->BroadcastMessage(::protocol::MESSAGE_COMBAT_ACTION_SYNC, message);

  MYSYA_DEBUG("combat_field->BroadcastMessage MESSAGE_COMBAT_ACTION_SYNC.");
}

void CombatEventHandler::EventCombatMoveAction(const ProtoMessage *data) {
  const event::EventCombatMoveAction *event = (const event::EventCombatMoveAction *)data;

  CombatField *combat_field =
    CombatFieldManager::GetInstance()->Get(event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  combat_field->PushAction(event->action());

  ::protocol::MessageCombatActionSync message;
  *message.mutable_action() = event->action();
  combat_field->BroadcastMessage(::protocol::MESSAGE_COMBAT_ACTION_SYNC, message);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
