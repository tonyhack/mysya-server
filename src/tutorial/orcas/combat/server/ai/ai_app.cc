#include "tutorial/orcas/combat/server/ai/ai_app.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/require_dispatcher.h"
#include "tutorial/orcas/combat/server/ai/vote_handler.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

MYSYA_SINGLETON_IMPL(AiApp);

AiApp::AiApp()
  : host_(NULL) {}
AiApp::~AiApp() {}

bool AiApp::Initialize(AppServer *host) {
  this->host_ = host;

  if (this->combat_event_handler_.Initialize() == false) {
    MYSYA_ERROR("[AI] CombatEventHandler::Initialize failed.");
    return false;
  }

  if (this->require_handler_.Initialize() == false) {
    MYSYA_ERROR("[AI] RequireHandler::Initialize failed.");
    return false;
  }

  if (this->scene_event_handler_.Initialize() == false) {
    MYSYA_ERROR("[AI] SceneEventHandler::Initialize failed.");
    return false;
  }

  if (this->vote_handler_.Initialize() == false) {
    MYSYA_ERROR("[AI] VoteHandler::Initialize() failed.");
    return false;
  }

  return true;
}

void AiApp::Finalize() {
  this->combat_event_handler_.Finalize();
  this->require_handler_.Finalize();
  this->scene_event_handler_.Finalize();
  this->vote_handler_.Finalize();

  this->host_ = NULL;
}

AppServer *AiApp::GetHost() {
  return this->host_;
}

EventDispatcher *AiApp::GetEventDispatcher() {
  return this->host_->GetEventDispatcher();
}

RequireDispatcher *AiApp::GetRequireDispatcher() {
  return this->host_->GetRequireDispatcher();
}

VoteDispatcher *AiApp::GetVoteDispatcher() {
  return this->host_->GetVoteDispatcher();
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
