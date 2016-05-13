#include "tutorial/orcas/combat/server/combat_warrior_field_pool.h"

#include "tutorial/orcas/combat/server/configs.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

MYSYA_SINGLETON_IMPL(CombatWarriorFieldPool);

CombatWarriorFieldPool::CombatWarriorFieldPool()
  : pool_(Configs::GetInstance()->combat_warrior_initial_size_,
      Configs::GetInstance()->combat_warrior_extend_size_) {}
CombatWarriorFieldPool::~CombatWarriorFieldPool() {}

CombatWarriorField *CombatWarriorFieldPool::Allocate() {
  return this->pool_.Allocate();
}

void CombatWarriorFieldPool::Deallocate(CombatWarriorField *warrior) {
  this->pool_.Deallocate(warrior);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
