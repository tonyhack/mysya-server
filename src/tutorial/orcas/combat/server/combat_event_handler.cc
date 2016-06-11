#include "tutorial/orcas/combat/server/combat_event_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_role_field_manager.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
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
    event_token_convert_camp_(0),
    event_token_building_switch_status_(0) {}

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
  this->event_token_building_switch_status_ =
    this->host_->GetEventDispatcher()->Attach(event::EVENT_COMBAT_BUILDING_SWITCH_STATUS, std::bind(
          &CombatEventHandler::OnEventCombatBuildingSwitchStatus, this, std::placeholders::_1));
}

void CombatEventHandler::ResetHandlers() {
  this->host_->GetEventDispatcher()->Detach(this->event_token_move_action_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_build_action_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_death_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_lock_target_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_convert_camp_);
  this->host_->GetEventDispatcher()->Detach(this->event_token_building_switch_status_);
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

  ::protocol::CombatAction action;
  action.set_type(::protocol::COMBAT_ACTION_TYPE_DEATH);
  action.set_timestamp(combat_field->GetTimestampMsec());

  ::protocol::CombatDeathAction *death_action = action.mutable_death_action();
  *death_action->mutable_host() = event->target();

  combat_field->PushAction(action);

  ::protocol::MessageCombatActionSync message;
  *message.mutable_action() = action;
  combat_field->BroadcastMessage(::protocol::MESSAGE_COMBAT_ACTION_SYNC, message);

  if (event->target().type() == ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    CombatWarriorField *combat_warrior_field = combat_field->GetWarrior(event->target().id());
    if (combat_warrior_field == NULL) {
      MYSYA_ERROR("CombatField::GetWarrior(%d) failed.",
          event->target().id());
      return;
    }

    CombatRoleField *combat_role_field = combat_warrior_field->GetRoleField();
    if (combat_role_field == NULL) {
      MYSYA_ERROR("CombatWarriorField::GetRoleField() failed.");
      return;
    }

    combat_role_field->IncSupply(0 - combat_warrior_field->GetFields().supply_need());

    combat_field->PrintRoleResources();
  }
}

void CombatEventHandler::OnEventCombatLockTarget(const ProtoMessage *data) {
  const event::EventCombatLockTarget *event = (const event::EventCombatLockTarget *)data;

  CombatField *combat_field =
    CombatFieldManager::GetInstance()->Get(event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  ::protocol::CombatAction action;
  action.set_type(::protocol::COMBAT_ACTION_TYPE_LOCK_TARGET);
  action.set_timestamp(combat_field->GetTimestampMsec());

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

  CombatRoleField *combat_role_field =
    CombatRoleFieldManager::GetInstance()->Get(event->host_id());
  if (combat_role_field == NULL) {
    MYSYA_ERROR("CombatRoleFieldManager::Get(%d) failed.", event->host_id());
    return;
  }

  CombatRoleField *original_combat_role_field =
    CombatRoleFieldManager::GetInstance()->Get(event->original_host_id());
  if (original_combat_role_field == NULL) {
    MYSYA_ERROR("CombatRoleFieldManager::Get(%d) failed.", event->original_host_id());
    return;
  }

  if (event->host().type() == ::protocol::COMBAT_ENTITY_TYPE_BUILDING) {
    combat_role_field->SetBuildingNum(combat_role_field->GetBuildingNum() + 1);
    original_combat_role_field->SetBuildingNum(
        original_combat_role_field->GetBuildingNum() - 1);

    combat_field->AllocateBuildingSupply();
  } else if (event->host().type() == ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    CombatWarriorField *combat_warrior_field = combat_field->GetWarrior(event->host().id());
    if (combat_warrior_field == NULL) {
      MYSYA_ERROR("CombatField::GetWarrior(%d) failed.",
          event->host().id());
      return;
    }

    combat_role_field->IncSupply(combat_warrior_field->GetFields().supply_need());
    original_combat_role_field->IncSupply(0 - combat_warrior_field->GetFields().supply_need());

    combat_field->PrintRoleResources();
  } else {
    MYSYA_ERROR("EventCombatConvertCamp::host's type(%d) invalid.", event->host().type());
    return;
  }

  ::protocol::CombatAction action;
  action.set_type(::protocol::COMBAT_ACTION_TYPE_CONVERT_CAMP);
  action.set_timestamp(combat_field->GetTimestampMsec());

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

void CombatEventHandler::OnEventCombatBuildingSwitchStatus(const ProtoMessage *data) {
  const event::EventCombatBuildingSwitchStatus *event =
    (const event::EventCombatBuildingSwitchStatus *)data;

  CombatField *combat_field = CombatFieldManager::GetInstance()->Get(
      event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  CombatBuildingField *combat_building_field = combat_field->GetBuilding(event->building_id());
  if (combat_building_field == NULL) {
    MYSYA_ERROR("CombatField::GetBuilding(%d) failed.", event->building_id());
    return;
  }

  ::protocol::CombatAction action;
  action.set_type(::protocol::COMBAT_ACTION_TYPE_BUILDING_SWITCH_STATUS);
  action.set_timestamp(combat_field->GetTimestampMsec());

  ::protocol::CombatBuildingSwitchStatusAction *building_switch_status_action =
    action.mutable_building_switch_status_action();
  building_switch_status_action->set_building_id(event->building_id());
  building_switch_status_action->set_status(combat_building_field->GetFields().status());

  combat_field->PushAction(action);

  ::protocol::MessageCombatActionSync message;
  *message.mutable_action() = action;
  combat_field->BroadcastMessage(::protocol::MESSAGE_COMBAT_ACTION_SYNC, message);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
