#include "tutorial/orcas/combat/server/ai/auto_status.h"

#include "tutorial/orcas/combat/server/ai/auto.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatus::AutoStatus(Auto *host)
  : host_(host) {}
AutoStatus::~AutoStatus() {}

void AutoStatus::Start() {}
void AutoStatus::Stop() {}

bool AutoStatus::GotoStatus(int status) {
  return this->host_->GotoStatus(status);
}

void AutoStatus::AttachEvent(int type, const EventCallback &cb) {
  this->event_cbs_[type] = cb;
}

void AutoStatus::DetachEvent(int type) {
  this->event_cbs_.erase(type);
}

void AutoStatus::DispatchEvent(int type, const ProtoMessage *data) {
  EventCallbackMap::iterator iter = this->event_cbs_.find(type);
  if (iter != this->event_cbs_.end()) {
    iter->second(data);
  }
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
