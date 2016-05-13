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

  bool Initialize(int32_t id, CombatField *host, int32_t camp_id,
      const ::protocol::BuildingDescription &description);
  void Finalize();

  int32_t GetId() const;

  CombatField *GetHost();
  ::protocol::CombatBuildingFields &GetFields();

 private:
  CombatField *host_;
  ::protocol::CombatBuildingFields fields_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatBuildingField);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_BUILDING_FIELD_H
