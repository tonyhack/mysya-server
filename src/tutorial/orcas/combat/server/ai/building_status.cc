#include "tutorial/orcas/combat/server/ai/building_status.h"

#include "tutorial/orcas/combat/server/ai/building.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

BuildingStatus::BuildingStatus(Building *host)
  : host_(host) {}
BuildingStatus::~BuildingStatus() {}

bool BuildingStatus::GotoStatus(int status) {
  return this->host_->GotoStatus(status);
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
