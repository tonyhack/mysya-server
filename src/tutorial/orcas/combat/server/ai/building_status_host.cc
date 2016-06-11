#include "tutorial/orcas/combat/server/ai/building_status_host.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/ai/building.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

BuildingStatusHost::BuildingStatusHost(Building *host)
  : BuildingStatus(host) {}
BuildingStatusHost::~BuildingStatusHost() {}

void BuildingStatusHost::Start() {}

void BuildingStatusHost::Stop() {}

::protocol::BuildingStatusType BuildingStatusHost::GetType() const {
  return ::protocol::BUILDING_STATUE_TYPE_HOST;
}

void BuildingStatusHost::OnRecoverHp() {}

void BuildingStatusHost::OnEvent(int type, const ProtoMessage *event) {
  switch (type) {
    case event::EVENT_COMBAT_CONVERT_CAMP:
      this->OnEventCombatConvertCamp(event);
      break;
    default:
      break;
  }
}

void BuildingStatusHost::OnEventCombatConvertCamp(const ProtoMessage *data) {
  event::EventCombatConvertCamp *event = (event::EventCombatConvertCamp *)data;

  if (event->host().type() != ::protocol::COMBAT_ENTITY_TYPE_BUILDING ||
      event->host().id() != this->host_->GetId()) {
    return;
  }

  this->GotoStatus(::protocol::BUILDING_STATUE_TYPE_RETRIEVE);
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
