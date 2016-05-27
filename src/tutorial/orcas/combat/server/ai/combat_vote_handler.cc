#include "tutorial/orcas/combat/server/ai/combat_vote_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

#define AI_APP \
    AiApp::GetInstance

CombatVoteHandler::CombatVoteHandler()
  : token_combat_move_action_(0) {}
CombatVoteHandler::~CombatVoteHandler() {}

#define VOTE_DISPATCHER \
    AI_APP()->GetHost()->GetVoteDispatcher

bool CombatVoteHandler::Initialize() {
  this->token_combat_move_action_ =
    VOTE_DISPATCHER()->Attach(vote::VOTE_COMBAT_MOVE_ACTION, std::bind(
          &CombatVoteHandler::OnVoteCombatMoveAction, this, std::placeholders::_1));

  return true;
}

void CombatVoteHandler::Finalize() {
  VOTE_DISPATCHER()->Detach(this->token_combat_move_action_);
}

#undef VOTE_DISPATCHER

int CombatVoteHandler::OnVoteCombatMoveAction(const ProtoMessage *data) {
  const vote::VoteCombatMoveAction *vote = (const vote::VoteCombatMoveAction *data);

  Auto *autoz = AutoManager::GetInstance()->Get(vote->combat_id(),
      vote->warrior_id());
  if (autoz == NULL) {
    MYSYA_ERROR("AutoManager::Get(%d) failed.", vote->combat_id());
    return vote::VoteCombatMoveAction::ResultCode::UNKOWN;
  }

  if (autoz->GetPresentStatus()->GetType() != AutoStatus::type::SEARCH) {
    return vote::VoteCombatMoveAction::ResultCode::INCORRECT_STATUS;
  }

  return 0;
}

#undef AI_APP

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
