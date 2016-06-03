#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_FORMULA_REQUIRE_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_FORMULA_REQUIRE_HANDLER_H

#include <stdint.h>

#include <mysya/util/class_util.h>

#include "tutorial/orcas/protocol/cc/combat.pb.h"

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatBuildingField;
class CombatWarriorField;

namespace formula {

class RequireHandler {
  typedef ::google::protobuf::Message ProtoMessage;

 public:
  RequireHandler();
  ~RequireHandler();

  bool Initialize();
  void Finalize();

 private:
  void SendEventCombatDeath(int32_t combat_id, int32_t opponent_warrior_id,
      const ::protocol::CombatEntity &target);
  void SendEventCombatConvertCamp(int32_t combat_id, int32_t camp_id,
      int32_t host_id, const ::protocol::CombatEntity &host);
  void SendEventCombatAttack(int32_t combat_id, int32_t warrior_id,
      const ::protocol::CombatEntity &target, int32_t damage);

  int FormulaDamage(CombatWarriorField *active, CombatWarriorField *passive);
  int FormulaDamage(CombatWarriorField *active, CombatBuildingField *passive);

  int OnRequireFormulaAttack(ProtoMessage *data);

  MYSYA_DISALLOW_COPY_AND_ASSIGN(RequireHandler);
};

}  // namespace formula
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_FORMULA_REQUIRE_HANDLER_H
