#include "tutorial/orcas/gateway/server/combat_actor_manager.h"

#include "tutorial/orcas/gateway/server/combat_actor.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

MYSYA_SINGLETON_IMPL(CombatActorManager);

CombatActorManager::CombatActorManager() {}
CombatActorManager::~CombatActorManager() {}

CombatActor *CombatActorManager::Get(const std::string &name) {
  CombatActor *combat_actor = NULL;

  CombatActorHashmap::iterator iter = this->actors_.find(name);
  if (iter != this->actors_.end()) {
    combat_actor = iter->second;
  }

  return combat_actor;
}

CombatActor *CombatActorManager::Allocate(const std::string &name) {
  if (this->Get(name) != NULL) {
    return NULL;
  }

  CombatActor *combat_actor = new (std::nothrow) CombatActor(name);
  if (combat_actor == NULL) {
    return NULL;
  }

  this->actors_.insert(std::make_pair(name, combat_actor));
  return combat_actor;
}

void CombatActorManager::Deallocate(CombatActor *actor) {
  this->actors_.erase(actor->GetName());
  delete actor;
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
