#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_WARRIOR_FIELD_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_WARRIOR_FIELD_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/protocol/cc/warrior.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatRoleField;

class CombatWarriorField {
 public:
  CombatWarriorField();
  ~CombatWarriorField();

  bool Initialize(int32_t id, CombatRoleField *host,
      const ::protocol::WarriorDescription &description);
  void Finalize();

  int32_t GetId() const;

  CombatRoleField *GetRoleField();
  ::protocol::CombatWarriorFields &GetFields();
  ::protocol::CombatWarriorServerFields &GetServerFields();

  void GenerateFields();

 private:
  CombatRoleField *host_;
  ::protocol::CombatWarriorFields fields_;
  ::protocol::CombatWarriorServerFields server_fields_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatWarriorField);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_WARRIOR_FIELD_H
