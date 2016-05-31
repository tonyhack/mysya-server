#include "tutorial/orcas/combat/server/formula/formula_app.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/require_dispatcher.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace formula {

MYSYA_SINGLETON_IMPL(FormulaApp);

FormulaApp::FormulaApp()
  : host_(NULL) {}
FormulaApp::~FormulaApp() {}

bool FormulaApp::Initialize(AppServer *host) {
  this->host_ = host;

  if (this->require_handler_.Initialize() == false) {
    MYSYA_ERROR("[FORMULA] RequireHandler::Initialize failed.");
    return false;
  }

  return true;
}

void FormulaApp::Finalize() {
  this->require_handler_.Finalize();
}

AppServer *FormulaApp::GetHost() {
  return this->host_;
}

EventDispatcher *FormulaApp::GetEventDispatcher() {
  return this->host_->GetEventDispatcher();
}

RequireDispatcher *FormulaApp::GetRequireDispatcher() {
  return this->host_->GetRequireDispatcher();
}

VoteDispatcher *FormulaApp::GetVoteDispatcher() {
  return this->host_->GetVoteDispatcher();
}

}  // namespace formula
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
