#include "tutorial/orcas/combat/server/scene/combat_require_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/require/cc/require.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_combat.pb.h"
#include "tutorial/orcas/combat/server/scene/building.h"
#include "tutorial/orcas/combat/server/scene/entity_builder.h"
#include "tutorial/orcas/combat/server/scene/scene.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"
#include "tutorial/orcas/combat/server/scene/scene_manager.h"
#include "tutorial/orcas/combat/server/scene/warrior.h"
#include "tutorial/orcas/protocol/cc/warrior.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

CombatRequireHandler::CombatRequireHandler() {}
CombatRequireHandler::~CombatRequireHandler() {}

bool CombatRequireHandler::Initialize() {
  SceneApp::GetInstance()->GetHost()->GetRequireDispatcher()->Attach(
      require::REQUIRE_COMBAT_BUILD_ACTION, std::bind(
        &CombatRequireHandler::OnRequireCombatBuildAction, this, std::placeholders::_1));
  SceneApp::GetInstance()->GetHost()->GetRequireDispatcher()->Attach(
      require::REQUIRE_COMBAT_MOVE_ACTION, std::bind(
        &CombatRequireHandler::OnRequireCombatMoveAction, this, std::placeholders::_1));

  return true;
}

void CombatRequireHandler::Finalize() {
  SceneApp::GetInstance()->GetHost()->GetRequireDispatcher()->Detach(
      require::REQUIRE_COMBAT_BUILD_ACTION);
  SceneApp::GetInstance()->GetHost()->GetRequireDispatcher()->Detach(
      require::REQUIRE_COMBAT_MOVE_ACTION);
}

int CombatRequireHandler::OnRequireCombatBuildAction(ProtoMessage *data) {
  require::RequireCombatBuildAction *message = (require::RequireCombatBuildAction *)data;

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

  Building *building = scene->GetBuilding(message->action().building_id());
  if (building == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetBuilding(%d) failed.",
        message->action().building_id());
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
    MYSYA_ERROR("Warrior::Initialize() failed.");
    return -1;
  }

  ::protocol::Position warrior_pos;
  if (scene->GetNearlyWalkablePos(building->GetPos(), warrior_pos) == false) {
    SceneApp::GetInstance()->GetEntityBuilder()->DeallocateWarrior(warrior);
    MYSYA_ERROR("Scene::GetNearlyWalkablePos() failed.");
    return -1;
  }

  combat_warrior_field->GetFields().set_origin_pos_x(warrior_pos.x());
  combat_warrior_field->GetFields().set_origin_pos_y(warrior_pos.y());

  if (scene->AddWarrior(warrior) == false) {
    warrior->Finalize();
    SceneApp::GetInstance()->GetEntityBuilder()->DeallocateWarrior(warrior);
    MYSYA_ERROR("Scene::AddWarrior() failed.");
    return -1;
  }

  return 0;
}

int CombatRequireHandler::OnRequireCombatMoveAction(ProtoMessage *data) {
  require::RequireCombatMoveAction *message = (require::RequireCombatMoveAction *)data;

  Scene *scene = SceneManager::GetInstance()->Get(message->combat_id());
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::Get(%d) failed.",
        message->combat_id());
    return -1;
  }

  for (int i = 0; i < message->action().warrior_id_size(); ++i) {
    Warrior *warrior = scene->GetWarrior(message->action().warrior_id(i));
    if (warrior == NULL) {
      MYSYA_ERROR("[SCENE] Scene::GetWarrior(%d) failed.",
          message->action().warrior_id(i));
      return -1;
    }

    MoveAction *move_action = warrior->GetMoveAction();
    if (move_action == NULL) {
      MYSYA_ERROR("[SCENE] Scene::GetMoveAction() failed.");
      return -1;
    }

    move_action->Start(message->action().pos());
  }

  return 0;
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
