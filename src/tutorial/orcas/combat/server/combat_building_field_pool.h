#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_BUILDING_FIELD_POOL_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_BUILDING_FIELD_POOL_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/pool_template.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatBuildingField;

class CombatBuildingFieldPool {
 public:
  CombatBuildingField *Allocate();
  void Deallocate(CombatBuildingField *building);

 private:
  PoolTemplate<CombatBuildingField> pool_;

  MYSYA_SINGLETON(CombatBuildingFieldPool);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_BUILDING_FIELD_POOL_H
