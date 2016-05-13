#include "tutorial/orcas/combat/server/scene/combat_require_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

CombatRequireHandler::CombatRequireHandler()
  : host_(NULL) {}
CombatRequireHandler::~CombatRequireHandler() {}

bool CombatRequireHandler::Initialize(SceneApp *host) {
  this->host_ = host;

  this->host_->GetRequireDispatcher()->Attach( require::REQUIRE_COMBAT_MOVE_ACTION,
      std::bind( &CombatRequireHandler::OnRequireCombatMoveAction, this,
        std::placeholders::_1));

  return true;
}

void CombatRequireHandler::Finalize() {
  this->host_->GetRequireDispatcher()->Detach(require::REQUIRE_COMBAT_MOVE_ACTION);

  this->host_ = NULL;
}

int CombatRequireHandler::OnRequireCombatMoveAction(ProtoMessage *data) {
  require::RequireCombatMoveAction *message =
    (require::RequireCombatMoveAction *)data;

  Scene *scene = SceneManager::GetInstance()->Get(message->combat_id());
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::Get(%d) failed.",
        message->combat_id());
    return -1;
  }

  Warrior *warrior = scene->GetWarrior(message->action().warrior_id());
  if (warrior == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetWarrior(%d) failed.",
        message->action().warrior_id());
    return -1;
  }

  MoveAction *move_action = warrior->GetMoveAction();
  if (move_action == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetMoveAction() failed.");
    return -1;
  }

  move_action->Start(message->action().pos());

  return 0;
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
