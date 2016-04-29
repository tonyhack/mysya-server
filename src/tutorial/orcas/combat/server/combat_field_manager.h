#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_FIELD_MANAGER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_FIELD_MANAGER_H

#include <stdint.h>

#include <unordered_map>

#include <mysya/util/class_util.h>
 
#include "tutorial/orcas/combat/pool_template.h"
#include "tutorial/orcas/util/id_allocator.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatField;

class CombatFieldManager {
 public:
  typedef std::unordered_map<int32_t, CombatField *> CombatHashmap;

  CombatField *Allocate();
  void Deallocate(CombatField *combat);

  bool Add(CombatField *combat);
  CombatField *Get(int32_t id);
  CombatField *Remove(int32_t id);

 private:
  CombatHashmap combats_;
  PoolTemplate<CombatField> pool_;
  ::tutorial::orcas::util::IdAllocator<int32_t> ids_;

  MYSYA_SINGLETON(CombatFieldManager);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_FIELD_MANAGER_H
