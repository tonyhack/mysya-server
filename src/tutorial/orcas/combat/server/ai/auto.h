#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_H

#include "tutorial/orcas/combat/server/ai/auto_manager.h"

#include "tutorial/orcas/combat/server/ai/auto_status.h"
#include "tutorial/orcas/combat/server/ai/auto_status_attack.h"
#include "tutorial/orcas/combat/server/ai/auto_status_chase.h"
#include "tutorial/orcas/combat/server/ai/auto_status_search.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class AutoStatus;
class CombatWarriorField;

class Auto {
 public:
  typedef AutoManager::AutoGlobalId AutoGlobalId;

  Auto();
  ~Auto();

  bool Initialize(CombatWarriorField *host);
  void Finalize();

  int32_t Getid() const;
  AutoGlobalId GetGlobalId() const;
  CombatWarriorField *GetHost();

  void SetTarget(::protocol::CombatEntityType type, int32_t id);
  ::protocol::CombatTarget &GetTarget();
  int GetTargetDistance() const;

  AutoStatus *GetPresentStatus();
  void GotoStatus(int status);

  bool MoveTarget();
  bool SearchTarget();
  bool AttackTarget();

 private:
  CombatWarriorField *host_;
  ::protocol::CombatTarget target_;

  AutoStatus *present_status_;
  AutoStatusAttack status_attack_;
  AutoStatusChase status_chase_;
  AutoStatusSearch status_search_;
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_H
