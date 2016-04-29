#include "tutorial/orcas/combat/server/combat_role_field_manager.h"

#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/configs.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

MYSYA_SINGLETON_IMPL(CombatRoleFieldManager);

CombatRoleFieldManager::CombatRoleFieldManager()
  : pool_(Configs::GetInstance()->combat_role_initial_size_,
      Configs::GetInstance()->combat_role_extend_size_) {}

CombatRoleFieldManager::~CombatRoleFieldManager() {
  for (RoleHashmap::iterator iter = this->roles_.begin();
      iter != this->roles_.end(); ++iter) {
    this->Deallocate(iter->second);
  }

  this->roles_.clear();
}

CombatRoleField *CombatRoleFieldManager::Allocate() {
  return this->pool_.Allocate();
}

void CombatRoleFieldManager::Deallocate(CombatRoleField *role) {
  this->pool_.Deallocate(role);
}

bool CombatRoleFieldManager::Add(CombatRoleField *role) {
  RoleHashmap::iterator iter = this->roles_.find(role->GetArgentId());
  if (iter != this->roles_.end()) {
    return false;
  }

  this->roles_.insert(std::make_pair(role->GetArgentId(), role));
  return true;
}

CombatRoleField *CombatRoleFieldManager::Get(uint64_t argent_id) {
  CombatRoleField *role = NULL;

  RoleHashmap::iterator iter = this->roles_.find(argent_id);
  if (iter != this->roles_.end()) {
    role = iter->second;
  }

  return role;
}

CombatRoleField *CombatRoleFieldManager::Remove(uint64_t argent_id) {
  CombatRoleField *role = NULL;

  RoleHashmap::iterator iter = this->roles_.find(argent_id);
  if (iter != this->roles_.end()) {
    role = iter->second;
    this->roles_.erase(iter);
  }

  return role;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
