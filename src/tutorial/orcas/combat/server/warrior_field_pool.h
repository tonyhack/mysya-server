#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_WARRIOR_FIELD_POOL_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_WARRIOR_FIELD_POOL_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/pool_template.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class WarriorField;

class WarriorFieldPool {
 public:
  WarriorField *Allocate();
  void Deallocate(WarriorField *warrior);

 private:
  PoolTemplate<WarriorField> pool_;

  MYSYA_SINGLETON(WarriorFieldPool);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_WARRIOR_FIELD_POOL_H
