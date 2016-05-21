#include "tutorial/orcas/gateway/server/combat_actor_manager.h"

#include "tutorial/orcas/gateway/server/combat_actor.h"
#include "tutorial/orcas/gateway/server/warrior_config.h"

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

  typedef WarriorConfig::WarriorHashmap WarriorHashmap;
  const WarriorHashmap &warrior_confs = WarriorConfig::GetInstance()->GetWarriors();
  for (WarriorHashmap::const_iterator iter = warrior_confs.begin();
      iter != warrior_confs.end(); ++iter) {
    ::protocol::WarriorDescription warrior_description;
    warrior_description.set_id(iter->second.id_);
    warrior_description.set_type(iter->second.type_);
    warrior_description.set_max_hp(iter->second.hp_);
    warrior_description.set_attack(iter->second.attack_);
    warrior_description.set_defence(iter->second.defence_);
    warrior_description.set_move_speed(iter->second.move_speed_);
    warrior_description.set_attack_speed(iter->second.attack_speed_);
    warrior_description.set_attack_range(iter->second.attack_range_);
    combat_actor->AddWarrior(warrior_description);
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
