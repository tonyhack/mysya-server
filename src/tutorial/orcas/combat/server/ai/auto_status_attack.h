#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_ATTACK_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_ATTACK_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/ai/auto_state.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class AutoStatusAttack : public AutoStatus {
 public:
  AutoStatusAttack(Auto *host);
  virtual ~AutoStatusAttack();

  virtual void Start();
  virtual void Stop();

 private:
  void SetAttackTimer();
  void ResetAttackTimer();

  void OnTimerAttack(int64_t timer_id);
  void OnEventCombatDeath(const ProtoMessage *data);

  int64_t timer_id_attack_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(AutoStatusAttack);
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_ATTACK_H
