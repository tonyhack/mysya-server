#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_WARRIOR_FIELD_POOL_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_WARRIOR_FIELD_POOL_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/pool_template.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatWarriorField;

class CombatWarriorFieldPool {
 public:
  CombatWarriorField *Allocate();
  void Deallocate(CombatWarriorField *warrior);

 private:
  PoolTemplate<CombatWarriorField> pool_;

  MYSYA_SINGLETON(CombatWarriorFieldPool);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_WARRIOR_FIELD_POOL_HR
