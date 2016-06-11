#include "tutorial/orcas/combat/server/scene/require_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/require/cc/require.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_combat.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_scene.pb.h"
#include "tutorial/orcas/combat/server/scene/building.h"
#include "tutorial/orcas/combat/server/scene/entity_builder.h"
#include "tutorial/orcas/combat/server/scene/scene.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"
#include "tutorial/orcas/combat/server/scene/scene_manager.h"
#include "tutorial/orcas/combat/server/scene/warrior.h"
#include "tutorial/orcas/combat/server/vote/cc/vote.pb.h"
#include "tutorial/orcas/combat/server/vote/cc/vote_combat.pb.h"
#include "tutorial/orcas/combat/server/vote/cc/vote_scene.pb.h"
#include "tutorial/orcas/protocol/cc/building.pb.h"
#include "tutorial/orcas/protocol/cc/warrior.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

#define SCENE_APP \
    SceneApp::GetInstance

RequireHandler::RequireHandler() {}
RequireHandler::~RequireHandler() {}

#define REQUIRE_DISPATCHER \
    SCENE_APP()->GetHost()->GetRequireDispatcher

bool RequireHandler::Initialize() {
  REQUIRE_DISPATCHER()->Attach(require::REQUIRE_SCENE_BUILD, std::bind(
        &RequireHandler::OnRequireSceneBuild, this, std::placeholders::_1));
  REQUIRE_DISPATCHER()->Attach(require::REQUIRE_SCENE_MOVE, std::bind(
        &RequireHandler::OnRequireSceneMove, this, std::placeholders::_1));
  REQUIRE_DISPATCHER()->Attach(require::REQUIRE_SCENE_FETCH, std::bind(
        &RequireHandler::OnRequireSceneFetch, this, std::placeholders::_1));
  REQUIRE_DISPATCHER()->Attach(require::REQUIRE_COMBAT_MOVE_PATHS, std::bind(
        &RequireHandler::OnRequireCombatMovePaths, this, std::placeholders::_1));

  return true;
}

void RequireHandler::Finalize() {
  REQUIRE_DISPATCHER()->Detach(require::REQUIRE_SCENE_BUILD);
  REQUIRE_DISPATCHER()->Detach(require::REQUIRE_SCENE_MOVE);
  REQUIRE_DISPATCHER()->Detach(require::REQUIRE_SCENE_FETCH);
  REQUIRE_DISPATCHER()->Detach(require::REQUIRE_COMBAT_MOVE_PATHS);
}

#undef REQUIRE_DISPATCHER

int RequireHandler::OnRequireSceneBuild(ProtoMessage *data) {
  require::RequireSceneBuild *message = (require::RequireSceneBuild *)data;

  // Vote.
  vote::VoteCombatBuild vote_message;
  vote_message.set_combat_id(message->combat_id());
  vote_message.set_building_id(message->building_id());
  vote_message.set_warrior_id(message->warrior_id());
  int result_code = SCENE_APP()->GetHost()->GetVoteDispatcher()->Dispatch(
      vote::VOTE_COMBAT_BUILD, &vote_message);
  if (result_code < 0) {
    MYSYA_ERROR("VOTE_SCENE_MOVE result_code(%d).", result_code);
    return result_code;
  }

  Scene *scene = SceneManager::GetInstance()->Get(message->combat_id());
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::Get(%d) failed.",
        message->combat_id());
    return -1;
  }

  CombatField *combat_field = CombatFieldManager::GetInstance()->Get(
      message->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("[SCENE] CombatFieldManager::Get(%d) failed.",
        message->combat_id());
    return -1;
  }

  Building *building = scene->GetBuilding(message->building_id());
  if (building == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetBuilding(%d) failed.",
        message->building_id());
    return -1;
  }

  int32_t warrior_id = message->warrior_id();

  CombatWarriorField *combat_warrior_field = combat_field->GetWarrior(
      warrior_id);
  if (combat_warrior_field == NULL) {
    MYSYA_ERROR("[SCENE] CombatField::GetWarrior(%d) failed.", warrior_id);
    return -1;
  }

  Warrior *warrior = SceneApp::GetInstance()->GetEntityBuilder()->AllocateWarrior();
  if (warrior == NULL) {
    MYSYA_ERROR("[SCENE] EntityBuilder::AllocateWarrior() failed.");
    return -1;
  }

  if (warrior->Initialize(combat_warrior_field, scene) == false) {
    SceneApp::GetInstance()->GetEntityBuilder()->DeallocateWarrior(warrior);
    MYSYA_ERROR("[SCENE] Warrior::Initialize() failed.");
    return -1;
  }

  ::protocol::Position warrior_pos;
  if (scene->GetNearlyWalkablePos(building->GetPos(), warrior_pos) == false) {
    SceneApp::GetInstance()->GetEntityBuilder()->DeallocateWarrior(warrior);
    MYSYA_ERROR("[SCENE] Scene::GetNearlyWalkablePos() failed.");
    return -1;
  }

  combat_warrior_field->GetFields().set_origin_pos_x(warrior_pos.x());
  combat_warrior_field->GetFields().set_origin_pos_y(warrior_pos.y());

  if (scene->AddWarrior(warrior) == false) {
    warrior->Finalize();
    SceneApp::GetInstance()->GetEntityBuilder()->DeallocateWarrior(warrior);
    MYSYA_ERROR("[SCENE] Scene::AddWarrior() failed.");
    return -1;
  }

  scene->PrintStatusImage();

  return 0;
}

int RequireHandler::OnRequireSceneMove(ProtoMessage *data) {
  require::RequireSceneMove *message = (require::RequireSceneMove *)data;

  // Vote.
  vote::VoteSceneMove vote_message;
  vote_message.set_combat_id(message->combat_id());
  vote_message.set_warrior_id(message->warrior_id());
  int result_code = SCENE_APP()->GetHost()->GetVoteDispatcher()->Dispatch(
      vote::VOTE_SCENE_MOVE, &vote_message);
  if (result_code < 0) {
    MYSYA_ERROR("VOTE_SCENE_MOVE result_code(%d).", result_code);
    return result_code;
  }

  Scene *scene = SceneManager::GetInstance()->Get(message->combat_id());
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::GetInstance()->Get(%d) failed.",
        message->combat_id());
    return -1;
  }

  Warrior *warrior = scene->GetWarrior(message->warrior_id());
  if (warrior == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetWarrior(%d) failed.",
        message->warrior_id());
    return -1;
  }

  MoveAction *move_action = warrior->GetMoveAction();
  if (move_action == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetMoveAction() failed.");
    return -1;
  }

  move_action->Start(message->dest_pos());

  return 0;
}

int RequireHandler::OnRequireSceneFetch(ProtoMessage *data) {
  require::RequireSceneFetch *message = (require::RequireSceneFetch *)data;

  Scene *scene = SceneManager::GetInstance()->Get(message->combat_id());
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::Get(%d) failed.", message->combat_id());
    return -1;
  }

  typedef Scene::BuildingVector BuildingVector;
  typedef Scene::WarriorVector WarriorVector;

  BuildingVector buildings;
  WarriorVector warriors;
  if (scene->GetNeighbors(message->pos(), message->range(),
        buildings, warriors) == false) {
    MYSYA_ERROR("[SCENE] Scene::GetNeighbors() failed.");
    return -1;
  }

  for (BuildingVector::iterator iter = buildings.begin();
      iter != buildings.end(); ++iter) {
    Building *building = *iter;
    if (building->GetHost()->GetFields().camp_id() ==
        message->except_camp_id()) {
      continue;
    }

    message->add_building(building->GetId());
  }

  for (WarriorVector::iterator iter = warriors.begin();
      iter != warriors.end(); ++iter) {
    Warrior *warrior = *iter;
    if (warrior->GetHost()->GetFields().camp_id() ==
        message->except_camp_id()) {
      continue;
    }

    message->add_warrior(warrior->GetId());
  }

  return 0;
}

int RequireHandler::OnRequireCombatMovePaths(ProtoMessage *data) {
  require::RequireCombatMovePaths *message = (require::RequireCombatMovePaths *)data;

  Scene *scene = SceneManager::GetInstance()->Get(message->combat_id());
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::Get(%d) failed.", message->combat_id());
    return -1;
  }

  Warrior *warrior = scene->GetWarrior(message->warrior_id());
  if (warrior == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetWarrior(%d) failed.", message->warrior_id());
    return -1;
  }

  MoveAction *move_action = warrior->GetMoveAction();
  if (move_action == NULL) {
    MYSYA_ERROR("[SCENE] Warrior::GetMoveAction() failed.");
    return -1;
  }

  if (move_action->GetMoveStatus() == false) {
    message->set_move_status(false);
    return 0;
  }

  int path_index = move_action->GetPathIndex();

  typedef MoveAction::PositionVector PositionVector;
  const PositionVector &paths = move_action->GetPaths();
  if (paths.size() <= (size_t)path_index) {
    MYSYA_ERROR("[SCENE] MoveAction::GetPaths() error.");
    return -1;
  }

  for (size_t i = path_index; i < paths.size(); ++i) {
    *message->add_path() = paths[i];
  }

  message->set_move_status(true);

  return 0;
}

#undef SCENE_APP

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
