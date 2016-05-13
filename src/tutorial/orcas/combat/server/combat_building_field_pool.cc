#include "tutorial/orcas/combat/server/combat_building_field_pool.h"

#include "tutorial/orcas/combat/server/configs.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

MYSYA_SINGLETON_IMPL(CombatBuildingFieldPool);

CombatBuildingFieldPool::CombatBuildingFieldPool()
  : pool_(Configs::GetInstance()->combat_building_initial_size_,
      Configs::GetInstance()->combat_building_extend_size_) {}
CombatBuildingFieldPool::~CombatBuildingFieldPool() {}

CombatBuildingField *CombatBuildingFieldPool::Allocate() {
  return this->pool_.Allocate();
}

void CombatBuildingFieldPool::Deallocate(CombatBuildingField *building) {
  this->pool_.Deallocate(building);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
