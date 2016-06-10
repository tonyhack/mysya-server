#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_BUILDING_FIELD_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_BUILDING_FIELD_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/protocol/cc/building.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatField;

class CombatBuildingField {
 public:
  CombatBuildingField();
  ~CombatBuildingField();

  bool Initialize(int32_t id, CombatField *host_combat, int64_t role_argent_id,
      int32_t camp_id, const ::protocol::BuildingDescription &description);
  void Finalize();

  int32_t GetId() const;

  int32_t GetFoodAdd() const;
  void SetFoodAdd(int32_t value);
  void IncFoodAdd(int32_t increment);

  int32_t GetSupply() const;
  void SetSupply(int32_t value);
  void IncSupply(int32_t increment);

  int32_t GetElixirAdd() const;
  void SetElixirAdd(int32_t value);
  void IncElixirAdd(int32_t increment);

  CombatField *GetCombatField();
  int32_t GetCombatRoleArgentId() const;
  ::protocol::CombatBuildingFields &GetFields();

 private:
  int64_t role_argent_id_;
  CombatField *host_combat_;
  ::protocol::CombatBuildingFields fields_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatBuildingField);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_BUILDING_FIELD_H
