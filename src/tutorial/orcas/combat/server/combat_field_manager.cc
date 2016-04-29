#include "tutorial/orcas/combat/server/combat_field_manager.h"

#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/configs.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

MYSYA_SINGLETON_IMPL(CombatFieldManager);

CombatFieldManager::CombatFieldManager()
  : pool_(Configs::GetInstance()->combat_initial_size_,
      Configs::GetInstance()->combat_extend_size_), ids_(0) {}

CombatFieldManager::~CombatFieldManager() {
  for (CombatHashmap::iterator iter = this->combats_.begin();
      iter != this->combats_.end(); ++iter) {
    this->Deallocate(iter->second);
  }

  this->combats_.clear();
}

CombatField *CombatFieldManager::Allocate() {
  CombatField *combat_field = this->pool_.Allocate();
  if (combat_field != NULL) {
    combat_field->SetId(this->ids_.Allocate());
  }

  return combat_field;
}

void CombatFieldManager::Deallocate(CombatField *combat) {
  this->ids_.Deallocate(combat->GetId());
  this->pool_.Deallocate(combat);
}

bool CombatFieldManager::Add(CombatField *combat) {
  CombatHashmap::iterator iter = this->combats_.find(combat->GetId());
  if (iter != this->combats_.end()) {
    return false;
  }

  this->combats_.insert(std::make_pair(combat->GetId(), combat));
  return true;
}

CombatField *CombatFieldManager::Get(int32_t id) {
  CombatField *combat = NULL;

  CombatHashmap::iterator iter = this->combats_.find(id);
  if (iter != this->combats_.end()) {
    combat = iter->second;
  }

  return combat;
}

CombatField *CombatFieldManager::Remove(int32_t id) {
  CombatField *combat = NULL;

  CombatHashmap::iterator iter = this->combats_.find(id);
  if (iter != this->combats_.end()) {
    combat = iter->second;
    this->combats_.erase(iter);
  }

  return combat;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
