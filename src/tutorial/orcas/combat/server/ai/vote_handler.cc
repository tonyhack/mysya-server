#include "tutorial/orcas/combat/server/ai/vote_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/vote_dispatcher.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/auto.h"
#include "tutorial/orcas/combat/server/ai/auto_manager.h"
#include "tutorial/orcas/combat/server/vote/cc/vote.pb.h"
#include "tutorial/orcas/combat/server/vote/cc/vote_scene.pb.h"

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
    VOTE_DISPATCHER()->Attach(vote::VOTE_SCENE_MOVE, std::bind(
          &VoteHandler::OnVoteSceneMove, this, std::placeholders::_1));

  return true;
}

void VoteHandler::Finalize() {
  VOTE_DISPATCHER()->Detach(this->token_scene_move_);
}

#undef VOTE_DISPATCHER

int VoteHandler::OnVoteSceneMove(const ProtoMessage *data) {
  const vote::VoteSceneMove *vote = (const vote::VoteSceneMove *)data;

  Auto *autoz = AutoManager::GetInstance()->Get(vote->combat_id(),
      vote->warrior_id());
  if (autoz == NULL) {
    MYSYA_ERROR("AutoManager::Get(%d) failed.", vote->combat_id());
    return vote::VoteSceneMove::UNKOWN;
  }

  if (autoz->GetPresentStatus()->GetType() != AutoStatus::type::SEARCH) {
    return vote::VoteSceneMove::INCORRECT_STATUS;
  }

  return 0;
}

#undef AI_APP

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
