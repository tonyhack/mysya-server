#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_ACTOR_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_ACTOR_H

#include <map>
#include <string>

#include "tutorial/orcas/protocol/cc/warrior.pb.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

class Actor;
class Combat;

class CombatActor {
 public:
  typedef std::map<int32_t, ::protocol::WarriorDescription> WarriorDescriptionMap;

  explicit CombatActor(const std::string name);
  ~CombatActor();

  Actor *GetActor();
  void SetActor(Actor *actor);

  // int32_t GetCombatId() const;
  // void SetCombatId(int32_t id);

  Combat *GetCombat();
  void SetCombat(Combat *combat);

  uint64_t GetCombatArgentId() const;
  int32_t GetCampId() const;
  void SetCampId(int value);

  const std::string &GetName() const;

  const WarriorDescriptionMap &GetWarriors() const;
  void AddWarrior(const ::protocol::WarriorDescription &warrior);
  const ::protocol::WarriorDescription *GetWarrior(int32_t id) const;

 private:
  void GeneratorCombatArgentId();

  static uint64_t g_argent_id_allocator_;

  Actor *actor_;
  // int32_t combat_id_;
  Combat *combat_;
  // 全局唯一的代理id
  uint64_t combat_argent_id_;
  // 阵营id
  int32_t camp_id_;
  std::string name_;
  WarriorDescriptionMap warriors_;
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_ACTOR_H
