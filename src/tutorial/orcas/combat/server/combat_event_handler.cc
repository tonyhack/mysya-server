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
    event_token_move_action_(0),
    event_token_death_(0),
    event_token_lock_target_(0),
    event_token_convert_camp_(0) {}
CombatEventHandler::~CombatEventHandler() {}

void CombatEventHandler::SetHandlers() {
  this->event_token_build_action_ =
    this->host_->GetEventDispatcher()->Attach(event::EVENT_COMBAT_BUILD_ACTION, std::bind(
          &CombatEventHandler::OnEventCombatBuildAction, this, std::placeholders::_1));
  this->event_token_move_action_ =
    this->host_->GetEventDispatcher()->Attach(event::EVENT_COMBAT_MOVE_ACTION, std::bind(
          &CombatEventHandler::OnEventCombatMoveAction, this, std::placeholders::_1));
  this->event_token_death_ =
    this->host_->GetEventDispatcher()->Attach(event::EVENT_COMBAT_DEATH, std::bind(
          &CombatEventHandler::OnEventCombatDeath, this, std::placeholders::_1));
  this->event_token_lock_target_ =
    this->host_->GetEventDispatcher()->Attach(event::EVENT_COMBAT_LOCK_TARGET, std::bind(
          &CombatEventHandler::OnEventCombatLockTarget, this, std::placeholders::_1));
  this->event_token_convert_camp_ =
    this->host_->GetEventDispatcher()->Attach(event::EVENT_COMBAT_CONVERT_CAMP, std::bind(
          &CombatEventHandler::OnEventCombatConvertCamp, this, std::placeholders::_1));
}

void CombatEventHandler::ResetHandlers() {
  this->host_->GetEventDispatcher()->Detach(this->event_token_move_action_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_build_action_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_death_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_lock_target_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_convert_camp_);
}

void CombatEventHandler::OnEventCombatBuildAction(const ProtoMessage *data) {
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

void CombatEventHandler::OnEventCombatMoveAction(const ProtoMessage *data) {
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

void CombatEventHandler::OnEventCombatDeath(const ProtoMessage *data) {
  const event::EventCombatDeath *event = (const event::EventCombatDeath *)data;

  CombatField *combat_field = CombatFieldManager::GetInstance()->Get(
      event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  const ::mysya::util::Timestamp &begin_timestamp = combat_field->GetBeginTimestamp();
  const ::mysya::util::Timestamp &now_timestamp = this->host_->GetTimestamp();

  ::protocol::CombatAction action;
  action.set_type(::protocol::COMBAT_ACTION_TYPE_DEATH);
  action.set_timestamp(now_timestamp.DistanceSecond(begin_timestamp));

  ::protocol::CombatDeathAction *death_action = action.mutable_death_action();
  *death_action->mutable_host() = event->target();

  combat_field->PushAction(action);

  ::protocol::MessageCombatActionSync message;
  *message.mutable_action() = action;
  combat_field->BroadcastMessage(::protocol::MESSAGE_COMBAT_ACTION_SYNC, message);
}

void CombatEventHandler::OnEventCombatLockTarget(const ProtoMessage *data) {
  const event::EventCombatLockTarget *event = (const event::EventCombatLockTarget *)data;

  CombatField *combat_field =
    CombatFieldManager::GetInstance()->Get(event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  const ::mysya::util::Timestamp &begin_timestamp = combat_field->GetBeginTimestamp();
  const ::mysya::util::Timestamp &now_timestamp = this->host_->GetTimestamp();

  ::protocol::CombatAction action;
  action.set_type(::protocol::COMBAT_ACTION_TYPE_LOCK_TARGET);
  action.set_timestamp(now_timestamp.DistanceSecond(begin_timestamp));

  ::protocol::CombatLockTargetAction *lock_target_action =
    action.mutable_lock_target_action();
  lock_target_action->set_warrior_id(event->warrior_id());
  *lock_target_action->mutable_target() = event->target();

  combat_field->PushAction(action);

  ::protocol::MessageCombatActionSync message;
  *message.mutable_action() = action;
  combat_field->BroadcastMessage(::protocol::MESSAGE_COMBAT_ACTION_SYNC, message);
}

void CombatEventHandler::OnEventCombatConvertCamp(const ProtoMessage *data) {
  const event::EventCombatConvertCamp *event = (const event::EventCombatConvertCamp *)data;

  CombatField *combat_field =
    CombatFieldManager::GetInstance()->Get(event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  const ::mysya::util::Timestamp &begin_timestamp = combat_field->GetBeginTimestamp();
  const ::mysya::util::Timestamp &now_timestamp = this->host_->GetTimestamp();

  ::protocol::CombatAction action;
  action.set_type(::protocol::COMBAT_ACTION_TYPE_CONVERT_CAMP);
  action.set_timestamp(now_timestamp.DistanceSecond(begin_timestamp));

  ::protocol::CombatConvertCampAction *convert_camp_action =
    action.mutable_convert_camp_action();
  *convert_camp_action->mutable_host() = event->host();
  convert_camp_action->set_camp_id(event->camp_id());
  convert_camp_action->set_host_id(event->host_id());

  combat_field->PushAction(action);

  ::protocol::MessageCombatActionSync message;
  *message.mutable_action() = action;
  combat_field->BroadcastMessage(::protocol::MESSAGE_COMBAT_ACTION_SYNC, message);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
