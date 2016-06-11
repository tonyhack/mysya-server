#include "tutorial/orcas/combat/server/ai/building_status_retrieve.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/ai/building.h"
#include "tutorial/orcas/protocol/cc/building.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

BuildingStatusRetrieve::BuildingStatusRetrieve(Building *host)
  : BuildingStatus(host) {}
BuildingStatusRetrieve::~BuildingStatusRetrieve() {}

void BuildingStatusRetrieve::Start() {}

void BuildingStatusRetrieve::Stop() {}

::protocol::BuildingStatusType BuildingStatusRetrieve::GetType() const {
  return ::protocol::BUILDING_STATUE_TYPE_RETRIEVE;
}

void BuildingStatusRetrieve::OnRecoverHp() {
  ::protocol::CombatBuildingFields &fields = this->host_->GetHost()->GetFields();
  if (fields.hp() >= fields.max_hp()) {
  this->GotoStatus(::protocol::BUILDING_STATUE_TYPE_HOST);
  }
}

void BuildingStatusRetrieve::OnEvent(int type, const ProtoMessage *event) {}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
