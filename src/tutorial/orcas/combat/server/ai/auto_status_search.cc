#include "tutorial/orcas/combat/server/ai/auto_status_search.h"

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
      std::bind(&AutoStatusSearch::OnTimerSearch, this, std::placehoder::_1));
}

void AutoStatusSearch::Stop() {
  AiApp::GetInstance()->GetHost()->StopTimer(this->timer_id_search_);
  this->timer_id_search_ = -1;
}

void AutoStatusSearch::OnTimerSearch(int64_t timer_id) {
  if (this->host_->SearchTarget() == true) {
    this->host_->GotoStatus(AutoStatus::CHASE);
  }
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
