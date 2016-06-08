#include "tutorial/orcas/combat/server/ai/require_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/require/cc/require.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_combat.pb.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/auto.h"
#include "tutorial/orcas/combat/server/ai/auto_manager.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

#define AI_APP \
    AiApp::GetInstance

RequireHandler::RequireHandler() {}
RequireHandler::~RequireHandler() {}

#define REQUIRE_DISPATCHER \
    AI_APP()->GetRequireDispatcher

bool RequireHandler::Initialize() {
  REQUIRE_DISPATCHER()->Attach(require::REQUIRE_COMBAT_ATTACKING_TARGET, std::bind(
        &RequireHandler::OnRequireCombatAttackingTarget, this, std::placeholders::_1));

  return true;
}

void RequireHandler::Finalize() {
  REQUIRE_DISPATCHER()->Detach(require::REQUIRE_COMBAT_ATTACKING_TARGET);
}

#undef REQUIRE_DISPATCHER

int RequireHandler::OnRequireCombatAttackingTarget(ProtoMessage *data) {
  require::RequireCombatAttackingTarget *message =
    (require::RequireCombatAttackingTarget *)data;

  Auto *autoz = AutoManager::GetInstance()->Get(message->combat_id(),
      message->warrior_id());
  if (autoz == NULL) {
    MYSYA_ERROR("[AI] AutoManager::Get(%d, %d) failed.",
        message->combat_id(), message->warrior_id());
    return -1;
  }

  if (autoz->GetPresentStatus()->GetType() != AutoStatus::ATTACK) {
    message->set_attack_status(false);
    return 0;
  }

  message->set_attack_status(true);
  *message->mutable_target() = autoz->GetTarget();
  return 0;
}

#undef AI_APP

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
