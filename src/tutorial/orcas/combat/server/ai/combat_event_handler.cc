#include "tutorial/orcas/combat/server/ai/combat_event_handler.h"

#include <functional>

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/auto.h"
#include "tutorial/orcas/combat/server/ai/auto_manager.h"
#include "tutorial/orcas/combat/server/ai/event_observer.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

#define AI_APP() \
    AiApp::GetInstance()

CombatEventHandler::CombatEventHandler()
  : event_token_build_action_(0),
    event_token_death_(0),
    event_token_convert_camp_(0),
    event_token_combat_attacked_(0) {}
CombatEventHandler::~CombatEventHandler() {}

#define EVENT_DISPATCHER \
    AI_APP()->GetHost()->GetEventDispatcher

bool CombatEventHandler::Initialize() {
  this->event_token_build_action_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_BUILD_ACTION, std::bind(
          &CombatEventHandler::OnEventCombatBuildAction, this, std::placeholders::_1));
  this->event_token_death_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_DEATH, std::bind(
          &CombatEventHandler::OnEventCombatDeath, this, std::placeholders::_1));
  this->event_token_convert_camp_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_CONVERT_CAMP, std::bind(
          &CombatEventHandler::OnEventCombatConvertCamp, this, std::placeholders::_1));
  this->event_token_combat_attacked_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_ATTACKED, std::bind(
          &CombatEventHandler::OnEventCombatAttacked, this, std::placeholders::_1));

  return true;

}

void CombatEventHandler::Finalize() {
  EVENT_DISPATCHER()->Detach(this->event_token_build_action_);
  EVENT_DISPATCHER()->Detach(this->event_token_death_);
  EVENT_DISPATCHER()->Detach(this->event_token_convert_camp_);
}

#undef EVENT_DISPATCHER

void CombatEventHandler::OnEventCombatBuildAction(const ProtoMessage *data) {
  const event::EventCombatBuildAction *event = (const event::EventCombatBuildAction *)data;

  CombatField *combat_field =
    CombatFieldManager::GetInstance()->Get(event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("[AI] CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  int32_t warrior_id = event->action().build_action().fields().id();
  CombatWarriorField *combat_warrior_field = combat_field->GetWarrior(warrior_id);
  if (combat_warrior_field == NULL) {
    MYSYA_ERROR("[AI] CombatField::GetWarrior(%d) failed.", warrior_id);
    return;
  }

  Auto *autoz = AutoManager::GetInstance()->Allocate();
  if (autoz == NULL) {
    MYSYA_ERROR("[AI] AutoManager;:Allocate() failed.");
    return;
  }

  if (autoz->Initialize(combat_warrior_field) == false) {
    MYSYA_ERROR("[AI] Auto::Initialize() failed.");
    AutoManager::GetInstance()->Deallocate(autoz);
    return;
  }

  if (AutoManager::GetInstance()->Add(autoz) == false) {
    MYSYA_ERROR("[AI] AutoManager::Add() failed.");
    autoz->Finalize();
    AutoManager::GetInstance()->Deallocate(autoz);
    return;
  }
}

void CombatEventHandler::OnEventCombatDeath(const ProtoMessage *data) {
  const event::EventCombatDeath *event = (const event::EventCombatDeath *)data;

  EventObserver::GetInstance()->Dispatch(event->combat_id(),
      event->target().id(), event::EVENT_COMBAT_DEATH, event);

  if (event->target().type() != ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    return;
  }

  Auto *autoz = AutoManager::GetInstance()->Remove(event->combat_id(),
      event->target().id());
  if (autoz == NULL) {
    MYSYA_ERROR("[AI] AutoManager::Remove(%d, %d) failed.",
        event->combat_id(), event->target().id());
    return;
  }

  autoz->Finalize();
  AutoManager::GetInstance()->Deallocate(autoz);
}

void CombatEventHandler::OnEventCombatConvertCamp(const ProtoMessage *data) {
  const event::EventCombatConvertCamp *event = (const event::EventCombatConvertCamp *)data;
  EventObserver::GetInstance()->Dispatch(event->combat_id(), event->host().id(),
      event::EVENT_COMBAT_CONVERT_CAMP, event);
}

void CombatEventHandler::OnEventCombatAttacked(const ProtoMessage *data) {
  const event::EventCombatAttacked *event = (const event::EventCombatAttacked *)data;
  if (event->host().type() == ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    EventObserver::GetInstance()->Dispatch(event->combat_id(), event->host().id(),
        event::EVENT_COMBAT_ATTACKED, event);
  }
}

#undef AI_APP

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
