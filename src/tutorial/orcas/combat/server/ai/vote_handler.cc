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

VoteHandler::VoteHandler()
  : token_scene_move_(0) {}
VoteHandler::~VoteHandler() {}

#define VOTE_DISPATCHER \
    AI_APP()->GetHost()->GetVoteDispatcher

bool VoteHandler::Initialize() {
  this->token_scene_move_ =
    VOTE_DISPATCHER()->Attach(vote::VOTE_COMBAT_MOVE_ACTION, std::bind(
          &VoteHandler::OnVoteSceneMove, this, std::placeholders::_1));

  return true;
}

void VoteHandler::Finalize() {
  VOTE_DISPATCHER()->Detach(this->token_scene_move_);
}

#undef VOTE_DISPATCHER

int VoteHandler::OnVoteSceneMove(const ProtoMessage *data) {
  const vote::VoteSceneMove *vote = (const vote::VoteSceneMove *data);

  Auto *autoz = AutoManager::GetInstance()->Get(vote->combat_id(),
      vote->warrior_id());
  if (autoz == NULL) {
    MYSYA_ERROR("AutoManager::Get(%d) failed.", vote->combat_id());
    return vote::VoteSceneMove::ResultCode::UNKOWN;
  }

  if (autoz->GetPresentStatus()->GetType() != AutoStatus::type::SEARCH) {
    return vote::VoteSceneMove::ResultCode::INCORRECT_STATUS;
  }

  return 0;
}

#undef AI_APP

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
