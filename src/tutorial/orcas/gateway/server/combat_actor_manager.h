#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_ACTOR_MANAGER_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_ACTOR_MANAGER_H

#include <string>
#include <unordered_map>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

class CombatActor;

class CombatActorManager {
 public:
  typedef std::unordered_map<std::string, CombatActor *> CombatActorHashmap;

  CombatActor *Get(const std::string &name);
  CombatActor *Allocate(const std::string &name);
  void Deallocate(CombatActor *actor);

 private:
  CombatActorHashmap actors_;

  MYSYA_SINGLETON(CombatActorManager);
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_ACTOR_MANAGER_H
