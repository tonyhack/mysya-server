#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_ROLE_FIELD_MANAGER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_ROLE_FIELD_MANAGER_H

#include <stdint.h>

#include <unordered_map>

#include <mysya/util/class_util.h>
 
#include "tutorial/orcas/combat/pool_template.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatRoleField;

class CombatRoleFieldManager {
 public:
  typedef std::unordered_map<uint64_t, CombatRoleField *> RoleHashmap;

  CombatRoleField *Allocate();
  void Deallocate(CombatRoleField *role);

  bool Add(CombatRoleField *role);
  CombatRoleField *Get(uint64_t argent_id);
  CombatRoleField *Remove(uint64_t argent_id);

 private:
  RoleHashmap roles_;
  PoolTemplate<CombatRoleField> pool_;

  MYSYA_SINGLETON(CombatRoleFieldManager);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_ROLE_FIELD_MANAGER_H
