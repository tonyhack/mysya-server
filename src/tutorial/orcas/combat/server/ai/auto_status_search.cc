#include "tutorial/orcas/combat/server/ai/auto_status_search.h"

#include <functional>

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/auto.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusSearch::AutoStatusSearch(Auto *host)
  : AutoStatus(host), timer_id_search_(-1) {}
AutoStatusSearch::~AutoStatusSearch() {}

void AutoStatusSearch::Start() {
  this->timer_id_search_ =
    AiApp::GetInstance()->GetHost()->StartTimer(kSearchExpireMsec_,
      std::bind(&AutoStatusSearch::OnTimerSearch, this, std::placeholders::_1));
}

void AutoStatusSearch::Stop() {
  AiApp::GetInstance()->GetHost()->StopTimer(this->timer_id_search_);
  this->timer_id_search_ = -1;
}

AutoStatus::type AutoStatusSearch::GetType() const {
  return AutoStatus::SEARCH;
}

void AutoStatusSearch::OnTimerSearch(int64_t timer_id) {
  if (this->host_->SearchTarget() == true) {
    this->GotoStatus(AutoStatus::CHASE);
  }
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
