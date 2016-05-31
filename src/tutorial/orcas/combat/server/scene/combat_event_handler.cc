#include "tutorial/orcas/combat/server/scene/combat_event_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"
#include "tutorial/orcas/combat/server/scene/building.h"
#include "tutorial/orcas/combat/server/scene/scene.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"
#include "tutorial/orcas/combat/server/scene/scene_manager.h"
#include "tutorial/orcas/combat/server/scene/warrior.h"
#include "tutorial/orcas/protocol/cc/building.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

CombatEventHandler::CombatEventHandler()
  : event_token_begin_(0),
    event_token_death_(0),
    event_token_lock_target_(0) {}

CombatEventHandler::~CombatEventHandler() {}

#define SCENE_APP \
    SceneApp::GetInstance

bool CombatEventHandler::Initialize() {
  this->event_token_begin_ = SCENE_APP()->GetEventDispatcher()->Attach(
      event::EVENT_COMBAT_BEGIN, std::bind(&CombatEventHandler::OnEventCombatBegin,
        this, std::placeholders::_1));
  this->event_token_death_ = SCENE_APP()->GetEventDispatcher()->Attach(
      event::EVENT_COMBAT_DEATH, std::bind(&CombatEventHandler::OnEventCombatDeath,
        this, std::placeholders::_1));
  this->event_token_lock_target_ = SCENE_APP()->GetEventDispatcher()->Attach(
      event::EVENT_COMBAT_LOCK_TARGET, std::bind(&CombatEventHandler::OnEventCombatLockTarget,
        this, std::placeholders::_1));

  return true;
}

void CombatEventHandler::Finalize() {
  SCENE_APP()->GetEventDispatcher()->Detach(this->event_token_begin_);
  SCENE_APP()->GetEventDispatcher()->Detach(this->event_token_death_);
  SCENE_APP()->GetEventDispatcher()->Detach(this->event_token_lock_target_);
}

void CombatEventHandler::OnEventCombatBegin(const ProtoMessage *data) {
  const event::EventCombatBegin *event = (const event::EventCombatBegin *)data;

  CombatField *combat_field =
    CombatFieldManager::GetInstance()->Get(event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("[SCENE] CombatFieldManager::Get(%d) failed.",
        event->combat_id());
    return;
  }

  Scene *scene = SceneManager::GetInstance()->Allocate(
      combat_field->GetMapId());
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::Allocate() failed.");
    return;
  }

  if (scene->Initialize(combat_field) == false) {
    MYSYA_ERROR("[SCENE] Scene::Initialize() failed.");
    SceneManager::GetInstance()->Deallocate(scene);
    return;
  }

  typedef CombatField::BuildingFieldMap BuildingFieldMap;
  const BuildingFieldMap &buildings = combat_field->GetBuildings();
  for (BuildingFieldMap::const_iterator iter = buildings.begin();
      iter != buildings.end(); ++iter) {
    Building *building =
      SCENE_APP()->GetEntityBuilder()->AllocateBuilding();
    if (building == NULL) {
      MYSYA_ERROR("[SCENE] EntityBuilder::AllocateBuilding() failed.");
      scene->Finalize();
      SceneManager::GetInstance()->Deallocate(scene);
      return;
    }

    if (building->Initialize(iter->second, scene) == false) {
      MYSYA_ERROR("[SCENE] Building::Initialize() failed.");
      SCENE_APP()->GetEntityBuilder()->DeallocateBuilding(building);
      scene->Finalize();
      SceneManager::GetInstance()->Deallocate(scene);
      return;
    }

    if (scene->AddBuilding(building) == false) {
      MYSYA_ERROR("[SCENE] Scene::AddBuilding() failed.");
      building->Finalize();
      SCENE_APP()->GetEntityBuilder()->DeallocateBuilding(building);
      scene->Finalize();
      SceneManager::GetInstance()->Deallocate(scene);
      return;
    }
  }

  if (SceneManager::GetInstance()->Add(scene) == false) {
    MYSYA_ERROR("[SCENE] SceneManager::Add(%d) failed.", scene->GetId());
    scene->Finalize();
    SceneManager::GetInstance()->Deallocate(scene);
    return;
  }
}

void CombatEventHandler::OnEventCombatDeath(const ProtoMessage *data) {
  const event::EventCombatDeath *event = (const event::EventCombatDeath *)data;

  CombatField *combat_field =
    CombatFieldManager::GetInstance()->Get(event->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("[SCENE] CombatFieldManager::Get(%d) failed.",
        event->combat_id());
    return;
  }

  Scene *scene = SceneManager::GetInstance()->Get(event->combat_id());
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::Get(%d) failed.", event->combat_id());
    return;
  }

  if (event->target().type() == ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    Warrior *warrior = scene->RemoveWarrior(event->target().id());
    if (warrior == NULL) {
      MYSYA_ERROR("[SCENE] Scene::RemoveWarrior(%d) failed.",
          event->target().id());
      return;
    }

    warrior->Finalize();
    SCENE_APP()->GetEntityBuilder()->DeallocateWarrior(warrior);
  } else if (event->target().type() == ::protocol::COMBAT_ENTITY_TYPE_BUILDING) {
    // TODO:
  } else {
    return;
  }

  scene->PrintStatusImage();
}

void CombatEventHandler::OnEventCombatLockTarget(const ProtoMessage *data) {
  const event::EventCombatLockTarget *event = (const event::EventCombatLockTarget *)data;

  Scene *scene = SceneManager::GetInstance()->Get(event->combat_id());
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::Get(%d) failed.", event->combat_id());
    return;
  }

  Warrior *warrior = scene->GetWarrior(event->warrior_id());
  if (warrior == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetWarrior(%d) failed.", event->warrior_id());
    return;
  }

  MoveAction *move_action = warrior->GetMoveAction();
  move_action->Reset();

  // TODO: notify client move reset.
}

#undef SCENE_APP

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
