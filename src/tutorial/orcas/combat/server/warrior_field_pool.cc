#include "tutorial/orcas/combat/server/warrior_field_pool.h"

#include "tutorial/orcas/combat/server/configs.h"
#include "tutorial/orcas/combat/server/warrior_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

MYSYA_SINGLETON_IMPL(WarriorFieldPool);

WarriorFieldPool::WarriorFieldPool()
  : pool_(Configs::GetInstance()->combat_warrior_initial_size_,
      Configs::GetInstance()->combat_warrior_extend_size_) {}
WarriorFieldPool::~WarriorFieldPool() {}

WarriorField *WarriorFieldPool::Allocate() {
  return this->pool_.Allocate();
}

void WarriorFieldPool::Deallocate(WarriorField *warrior) {
  this->pool_.Deallocate(warrior);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
