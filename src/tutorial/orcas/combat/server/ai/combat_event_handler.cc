#include "tutorial/orcas/combat/server/ai/combat_event_handler.h"

#include <functional>

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_role_field_manager.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/auto.h"
#include "tutorial/orcas/combat/server/ai/auto_manager.h"
#include "tutorial/orcas/combat/server/ai/building.h"
#include "tutorial/orcas/combat/server/ai/building_manager.h"
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
  : event_token_begin_(0),
    event_token_build_action_(0),
    event_token_death_(0),
    event_token_convert_camp_(0),
    event_token_attacked_(0),
    event_token_settle_(0),
    event_token_resource_recover_(0) {}
CombatEventHandler::~CombatEventHandler() {}

#define EVENT_DISPATCHER \
    AI_APP()->GetHost()->GetEventDispatcher

bool CombatEventHandler::Initialize() {
  this->event_token_begin_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_BEGIN, std::bind(
          &CombatEventHandler::OnEventCombatBegin, this, std::placeholders::_1));
  this->event_token_build_action_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_BUILD_ACTION, std::bind(
          &CombatEventHandler::OnEventCombatBuildAction, this, std::placeholders::_1));
  this->event_token_death_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_DEATH, std::bind(
          &CombatEventHandler::OnEventCombatDeath, this, std::placeholders::_1));
  this->event_token_convert_camp_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_CONVERT_CAMP, std::bind(
          &CombatEventHandler::OnEventCombatConvertCamp, this, std::placeholders::_1));
  this->event_token_attacked_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_ATTACKED, std::bind(
          &CombatEventHandler::OnEventCombatAttacked, this, std::placeholders::_1));
  this->event_token_settle_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_SETTLE, std::bind(
          &CombatEventHandler::OnEventCombatSettle, this, std::placeholders::_1));
  this->event_token_resource_recover_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_COMBAT_RESOURCE_RECOVER, std::bind(
          &CombatEventHandler::OnEventCombatResourceRecover, this, std::placeholders::_1));

  return true;

}

void CombatEventHandler::Finalize() {
  EVENT_DISPATCHER()->Detach(this->event_token_begin_);
  EVENT_DISPATCHER()->Detach(this->event_token_build_action_);
  EVENT_DISPATCHER()->Detach(this->event_token_death_);
  EVENT_DISPATCHER()->Detach(this->event_token_convert_camp_);
  EVENT_DISPATCHER()->Detach(this->event_token_settle_);
  EVENT_DISPATCHER()->Detach(this->event_token_resource_recover_);
}

#undef EVENT_DISPATCHER
void CombatEventHandler::OnEventCombatBegin(const ProtoMessage *data) {
  const event::EventCombatBegin *event = (const event::EventCombatBegin *)data;

  CombatField *combat_field =
    CombatFieldManager::GetInstance()->Get(event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("[AI] CombatFieldManager::Get(%d) failed.",
        event->combat_id());
    return;
  }

  typedef CombatField::BuildingFieldMap BuildingFieldMap;
  const BuildingFieldMap &buildings = combat_field->GetBuildings();
  for (BuildingFieldMap::const_iterator iter = buildings.begin();
      iter != buildings.end(); ++iter) {
    CombatBuildingField *combat_building_field = iter->second;

    Building *building = BuildingManager::GetInstance()->Allocate();
    if (building == NULL) {
      MYSYA_ERROR("[AI] BuildingManager::Allocate() failed.");
      return;
    }

    if (building->Initialize(combat_building_field) == false) {
      MYSYA_ERROR("[AI] Building::Initialize() failed.");
      BuildingManager::GetInstance()->Deallocate(building);
      return;
    }

    if (BuildingManager::GetInstance()->Add(building) == false) {
      MYSYA_ERROR("[AI] BuildingManager::Add() failed.");
      building->Finalize();
      BuildingManager::GetInstance()->Deallocate(building);
      return;
    }
  }
}

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

  if (event->host().type() == ::protocol::COMBAT_ENTITY_TYPE_BUILDING) {
    Building *building = BuildingManager::GetInstance()->Get(
        event->combat_id(), event->host().id());
    if (building == NULL) {
      MYSYA_ERROR("[AI] BuildingManager::Get(%d, %d) failed.",
          event->combat_id(), event->host().id());
      return;
    }

    building->GetPresentStatus()->OnEvent(event::EVENT_COMBAT_CONVERT_CAMP, event);
  }
}

void CombatEventHandler::OnEventCombatAttacked(const ProtoMessage *data) {
  const event::EventCombatAttacked *event = (const event::EventCombatAttacked *)data;
  if (event->host().type() == ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    EventObserver::GetInstance()->Dispatch(event->combat_id(), event->host().id(),
        event::EVENT_COMBAT_ATTACKED, event);
  }
}

void CombatEventHandler::OnEventCombatSettle(const ProtoMessage *data) {
  const event::EventCombatSettle *event = (const event::EventCombatSettle *)data;

  CombatField *combat_field = CombatFieldManager::GetInstance()->Get(
      event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("[AI] CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  typedef CombatField::WarriorFieldHashmap WarriorFieldHashmap;

  const WarriorFieldHashmap &warriors = combat_field->GetWarriors();
  for (WarriorFieldHashmap::const_iterator iter = warriors.begin();
      iter != warriors.end(); ++iter) {
    CombatWarriorField *combat_warrior_field = iter->second;

    Auto *autoz = AutoManager::GetInstance()->Remove(event->combat_id(),
        combat_warrior_field->GetId());
    if (autoz == NULL) {
      MYSYA_ERROR("[AI] AutoManager::Remove(%d, %d) failed.",
        event->combat_id(), combat_warrior_field->GetId());
      return;
    }

    autoz->Finalize();
    AutoManager::GetInstance()->Deallocate(autoz);
  }
}

void CombatEventHandler::OnEventCombatResourceRecover(const ProtoMessage *data) {
  const event::EventCombatResourceRecover *event =
    (const event::EventCombatResourceRecover *)data;

  CombatField *combat_field = CombatFieldManager::GetInstance()->Get(
      event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("[AI] CombatFieldManager::Get(%d) failed.", event->combat_id());
    return;
  }

  typedef CombatField::BuildingFieldMap BuildingFieldMap;
  BuildingFieldMap buildings = combat_field->GetBuildings();

  typedef std::map<int32_t, int32_t> CampFoodMap;
  typedef std::map<int32_t, int32_t> CampElixirMap;
  CampFoodMap camp_foods;
  CampElixirMap camp_elixirs;

  for (BuildingFieldMap::iterator building_iter = buildings.begin();
      building_iter != buildings.end(); ++building_iter) {
    CombatBuildingField *combat_building_field = building_iter->second;

    Building *building = BuildingManager::GetInstance()->Get(
        combat_field->GetId(), combat_building_field->GetId());
    if (building == NULL) {
      MYSYA_ERROR("[AI] BuildingManager::Get(%d, %d) failed.",
          combat_field->GetId(), combat_building_field->GetId());
      return;
    }

    if (building->GetTargetedNum() <= 0) {
      building->RecoveryHp();
    }

    int32_t camp_id = combat_building_field->GetFields().camp_id();

    CampFoodMap::iterator camp_food_iter = camp_foods.find(camp_id);
    if (camp_food_iter == camp_foods.end()) {
      camp_food_iter = camp_foods.insert(std::make_pair(camp_id, 0)).first;
    }

    camp_food_iter->second += combat_building_field->GetFields().food_add();

    CampElixirMap::iterator camp_elixir_iter = camp_elixirs.find(camp_id);
    if (camp_elixir_iter == camp_elixirs.end()) {
      camp_elixir_iter = camp_elixirs.insert(std::make_pair(camp_id, 0)).first;
    }

    camp_elixir_iter->second += combat_building_field->GetFields().elixir_add();
  }

  typedef CombatField::CombatRoleFieldSet CombatRoleFieldSet;
  CombatRoleFieldSet roles = combat_field->GetRoles();

  for (CombatRoleFieldSet::iterator role_iter = roles.begin();
      role_iter != roles.end(); ++role_iter) {
    CombatRoleField *combat_role_field =
      CombatRoleFieldManager::GetInstance()->Get(*role_iter);
    if (combat_role_field == NULL) {
      MYSYA_ERROR("CombatRoleFieldManager::Get(%lu) failed.", *role_iter);
      return;
    }

    CampFoodMap::iterator camp_food_iter = camp_foods.find(
        combat_role_field->GetCampId());
    if (camp_food_iter == camp_foods.end()) {
      MYSYA_ERROR("camp_id(%d) invalid.", combat_role_field->GetCampId());
      return;
    }
    combat_role_field->IncFood(camp_food_iter->second);

    CampElixirMap::iterator camp_elixir_iter = camp_elixirs.find(
        combat_role_field->GetCampId());
    if (camp_elixir_iter == camp_elixirs.end()) {
      MYSYA_ERROR("camp_id(%d) invalid.", combat_role_field->GetCampId());
      return;
    }
    combat_role_field->IncElixir(camp_elixir_iter->second);
  }

}

#undef AI_APP

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
