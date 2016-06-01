#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_H

#include "tutorial/orcas/combat/server/ai/auto_manager.h"

#include "tutorial/orcas/combat/server/ai/auto_status.h"
#include "tutorial/orcas/combat/server/ai/auto_status_attack.h"
#include "tutorial/orcas/combat/server/ai/auto_status_chase.h"
#include "tutorial/orcas/combat/server/ai/auto_status_search.h"
#include "tutorial/orcas/protocol/cc/combat.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatWarriorField;

namespace ai {

class AutoStatus;

class Auto {
 public:
  typedef AutoManager::AutoGlobalId AutoGlobalId;

  Auto();
  ~Auto();

  bool Initialize(CombatWarriorField *host);
  void Finalize();

  int32_t GetId() const;
  int32_t GetCombatId() const;
  AutoGlobalId GetGlobalId() const;
  CombatWarriorField *GetHost();

  void SetTarget(::protocol::CombatEntityType type, int32_t id);
  void ResetTarget();
  ::protocol::CombatEntity &GetTarget();
  int GetTargetDistance() const;

  AutoStatus *GetPresentStatus();
  bool GotoStatus(int status);

  bool MoveTarget();
  bool SearchTarget();
  bool AttackTarget();

 private:
  CombatWarriorField *host_;
  ::protocol::CombatEntity target_;

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
