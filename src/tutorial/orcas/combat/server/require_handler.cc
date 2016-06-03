#include "tutorial/orcas/combat/server/require_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/protocol/cc/combat_message.pb.h"
#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_role_field_manager.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_combat.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

#define REQUIRE_DISPATCHER \
    this->host_->GetRequireDispatcher

RequireHandler::RequireHandler(AppServer *host)
  : host_(host) {}
RequireHandler::~RequireHandler() {}

bool RequireHandler::SetHandlers() {
  REQUIRE_DISPATCHER()->Attach(require::REQUIRE_COMBAT_SETTLE, std::bind(
        &RequireHandler::OnRequireCombatSettle, this, std::placeholders::_1));

  return true;
}

void RequireHandler::ResetHandlers() {
  REQUIRE_DISPATCHER()->Detach(require::REQUIRE_COMBAT_SETTLE);
}

int RequireHandler::OnRequireCombatSettle(ProtoMessage *data) {
  require::RequireCombatSettle *message = (require::RequireCombatSettle *)data;

  CombatField *combat_field = CombatFieldManager::GetInstance()->Get(
      message->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", message->combat_id());
    return -1;
  }

  event::EventCombatSettle event;
  event.set_combat_id(message->combat_id());
  this->host_->GetEventDispatcher()->Dispatch(event::EVENT_COMBAT_SETTLE, &event);

  // settlement message.
  protocol::MessageCombatSettlementSync session_message;
  session_message.set_combat_id(combat_field->GetId());

  typedef CombatField::CombatRoleFieldSet CombatRoleFieldSet;
  const CombatRoleFieldSet &combat_roles = combat_field->GetRoles();
  for (CombatRoleFieldSet::const_iterator iter = combat_roles.begin();
      iter != combat_roles.end(); ++iter) {
    CombatRoleField *combat_role_field =
      CombatRoleFieldManager::GetInstance()->Get(*iter);
    if (combat_role_field != NULL) {
      ::protocol::CombatSettlementRole *settlement_role =
        session_message.mutable_settlement()->add_role();
      settlement_role->mutable_role_fields()->set_id(combat_role_field->GetArgentId());
      settlement_role->mutable_role_fields()->set_name(combat_role_field->GetName());
      settlement_role->mutable_role_fields()->set_camp_id(combat_role_field->GetCampId());
      settlement_role->set_building_num(combat_role_field->GetBuildingNum());
    }
  }

  // send settlement to session.
  combat_field->SendMessage(session_message);

  CombatFieldManager::GetInstance()->Remove(message->combat_id());
  combat_field->Finalize();
  CombatFieldManager::GetInstance()->Deallocate(combat_field);

  return 0;
}

#undef REQUIRE_DISPATCHER

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
