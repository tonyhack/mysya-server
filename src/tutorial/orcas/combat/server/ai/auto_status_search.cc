#include "tutorial/orcas/combat/server/ai/auto_status_search.h"

#include <functional>

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/auto.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusSearch::AutoStatusSearch(Auto *host)
  : AutoStatus(host), timer_id_search_(-1) {
  this->AttachEvent(event::EVENT_COMBAT_ATTACKED, std::bind(
        &AutoStatusSearch::OnEventCombatAttacked, this, std::placeholders::_1));
}
AutoStatusSearch::~AutoStatusSearch() {
  this->DetachEvent(event::EVENT_COMBAT_ATTACKED);
}

void AutoStatusSearch::Start() {
  this->timer_id_search_ =
    AiApp::GetInstance()->GetHost()->StartTimer(kSearchExpireMsec_,
      std::bind(&AutoStatusSearch::OnTimerSearch, this, std::placeholders::_1));
}

void AutoStatusSearch::Stop() {
  if (this->timer_id_search_ != -1) {
    AiApp::GetInstance()->GetHost()->StopTimer(this->timer_id_search_);
    this->timer_id_search_ = -1;
  }
}

AutoStatus::type AutoStatusSearch::GetType() const {
  return AutoStatus::SEARCH;
}

void AutoStatusSearch::OnTimerSearch(int64_t timer_id) {
  if (this->host_->SearchTarget() == true) {
    this->GotoStatus(AutoStatus::CHASE);
  }
}

void AutoStatusSearch::OnEventCombatAttacked(const ProtoMessage *data) {
  const event::EventCombatAttacked *event = (const event::EventCombatAttacked *)data;
  if (event->host().id() != this->host_->GetId()) {
    return;
  }

  this->host_->SetTarget(::protocol::COMBAT_ENTITY_TYPE_WARRIOR, event->warrior_id());
  this->GotoStatus(AutoStatus::CHASE);
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
