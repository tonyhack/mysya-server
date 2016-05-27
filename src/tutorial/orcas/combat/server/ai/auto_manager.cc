#include "tutorial/orcas/combat/server/ai/auto_manager.h"

#include "tutorial/orcas/combat/server/configs.h"
#include "tutorial/orcas/combat/server/ai/auto.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoManager::AutoManager()
  : pool_(Configs::GetInstance()->combat_warrior_initial_size_,
      Configs::GetInstance()->combat_warrior_extend_size_) {}
AutoManager::~AutoManager() {}

Auto *AutoManager::Allocate() {
  return this->pool_->Allocate();
}

void AutoManager::Deallocate(Auto *autoz) {
  this->pool_->Deallocate(autoz);
}

bool AutoManager::Add(Auto *autoz) {
  AutoHashmap::const_iterator iter = this->autoes_.find(
      autoz->GetGlobalId());
  if (iter != this->autoes_.end()) {
    return false;
  }

  this->autoes_.insert(std::make_pair(autoz->GetGlobalId(), autoz));
  return true;
}

Auto *AutoManager::Get(const AutoGlobalId &global_id) {
  Auto *autoz = NULL;

  AutoHashmap::iterator iter = this->autoes_.find(global_id);
  if (iter != this->autoes_.end()) {
    autoz = iter->second;
  }

  return autoz;
}

Auto *AutoManager::Get(int32_t combat_id, int32_t id) {
  return this->Get(AutoGlobalId(combat_id, id));
}

Auto *AutoManager::Remove(const AutoGlobalId &global_id) {
  Auto *autoz = NULL;

  AutoHashmap::iterator iter = this->autoes_.find(global_id);
  if (iter != this->autoes_.end()) {
    autoz = iter->second;
    this->autoes_.erase(iter);
  }

  return autoz;
}

Auto *AutoManager::Remove(int32_t combat_id, int32_t id) {
  return this->Remove(AutoGlobalId(combat_id, id));
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
