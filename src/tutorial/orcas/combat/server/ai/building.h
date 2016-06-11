#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_H

#include "tutorial/orcas/combat/server/ai/building_manager.h"
#include "tutorial/orcas/combat/server/ai/building_status_host.h"
#include "tutorial/orcas/combat/server/ai/building_status_retrieve.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatBuildingField;

namespace ai {

class BuildingStatus;

class Building {
 public:
  typedef BuildingManager::GlobalId GlobalId;

  Building();
  ~Building();

  bool Initialize(CombatBuildingField *host);
  void Finalize();

  int32_t GetId() const;
  int32_t GetCombatId() const;
  GlobalId GetGlobalId() const;
  CombatBuildingField *GetHost();

  int32_t GetTargetedNum() const;
  void IncTargetedNum();
  void DecTargetedNum();

  BuildingStatus *GetPresentStatus();
  bool GotoStatus(int status);

  void RecoveryHp();

 private:
  CombatBuildingField *host_;
  int32_t targeted_num_;
  int64_t timer_id_recovery_;

  BuildingStatus *present_status_;
  BuildingStatusHost status_host_;
  BuildingStatusRetrieve status_retrieve_;
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_H
