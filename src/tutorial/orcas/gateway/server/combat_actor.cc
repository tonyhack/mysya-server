#include "tutorial/orcas/gateway/server/combat_actor.h"

#include "tutorial/orcas/gateway/server/actor.h"
#include "tutorial/orcas/gateway/server/combat_manager.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

uint64_t CombatActor::g_argent_id_allocator_ = 0;

CombatActor::CombatActor(const std::string name)
  : actor_(NULL), combat_(NULL),
    combat_argent_id_(0), camp_id_(0),
    name_(name) {
  this->GeneratorCombatArgentId();
}
CombatActor::~CombatActor() {}

Actor *CombatActor::GetActor() {
  return this->actor_;
}

void CombatActor::SetActor(Actor *actor) {
  this->actor_ = actor;
}

// int32_t CombatActor::GetCombatId() const {
//   return this->combat_id_;
// }
// 
// void CombatActor::SetCombatId(int32_t id) {
//   this->combat_id_ = id;
// }

Combat *CombatActor::GetCombat() {
  return this->combat_;
}

void CombatActor::SetCombat(Combat *combat) {
  this->combat_ = combat;
}

uint64_t CombatActor::GetCombatArgentId() const {
  return this->combat_argent_id_;
}

int32_t CombatActor::GetCampId() const {
  return this->camp_id_;
}

void CombatActor::SetCampId(int value) {
  this->camp_id_ = value;
}

void CombatActor::GeneratorCombatArgentId() {
  this->combat_argent_id_ = ++CombatActor::g_argent_id_allocator_;
}

const std::string &CombatActor::GetName() const {
  return this->name_;
}

const CombatActor::WarriorDescriptionMap &CombatActor::GetWarriors() const {
  return this->warriors_;
}

void CombatActor::AddWarrior(const ::protocol::WarriorDescription &warrior) {
  this->warriors_.insert(std::make_pair(warrior.id(), warrior));
}

const ::protocol::WarriorDescription *CombatActor::GetWarrior(int32_t id) const {
  const ::protocol::WarriorDescription *warrior = NULL;

  WarriorDescriptionMap::const_iterator iter = this->warriors_.find(id);
  if (iter != this->warriors_.end()) {
    warrior = &iter->second;
  }

  return warrior;
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
