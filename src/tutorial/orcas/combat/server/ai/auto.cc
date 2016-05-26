#include "tutorial/orcas/combat/server/ai/auto.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

Auto::Auto()
  : host_(NULL), present_status_(NULL) {}
Auto::~Auto() {}

bool Auto::Initialize(CombatWarriorField *host) {
  this->host_ = host;
  this->present_status_ = &this->status_search_;
  this->target_.Clear();

  return true;
}

void Auto::Finalize() {
  this->target_.Clear();
  this->present_status_->Stop();
  this->present_status_ = NULL;
  this->host_ = NULL;
}

CombatWarriorField *Auto::GetHost() {
  return this->host_;
}

void Auto::SetTarget(::protocol::CombatEntityType type, int32_t id);
::protocol::CombatTarget &Auto::GetTarget();

AutoStatus *Auto::GetPresentStatus() {
  return this->present_status_;
}

void Auto::GotoStatus(int status) {}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
