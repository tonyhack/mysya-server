#include "tutorial/orcas/combat/server/ai/ai_app.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/configs.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/require_dispatcher.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

MYSYA_SINGLETON_IMPL(AiApp);

AiApp::AiApp()
  : host_(NULL) {}
AiApp::~AiApp() {}

bool AiApp::Initialize(AppServer *hosts) {
  this->host_ = host;

  return true;
}

void AiApp::Finalize() {
  this->host_ = NULL;
}

AppServer *AiApp::GetHost() {
  return this->host_;
}

EventDispatcher *AiApp::GetEventDispatcher() {
  return this->host_->GetEventDispatcher();
}

RequireDispatcher *GetRequireDispatcher() {
  return this->host_->GetRequireDispatcher();
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
