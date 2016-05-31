#include "tutorial/orcas/combat/server/formula/require_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"
#include "tutorial/orcas/combat/server/formula/formula_app.h"
#include "tutorial/orcas/combat/server/require/cc/require.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_formula.pb.h"
#include "tutorial/orcas/protocol/cc/building.pb.h"
#include "tutorial/orcas/protocol/cc/warrior.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace formula {

#define FORMULA_APP \
    FormulaApp::GetInstance

RequireHandler::RequireHandler() {}
RequireHandler::~RequireHandler() {}

#define REQUIRE_DISPATCHER \
    FORMULA_APP()->GetHost()->GetRequireDispatcher

bool RequireHandler::Initialize() {
  REQUIRE_DISPATCHER()->Attach(require::REQUIRE_FORMULA_ATTACK, std::bind(
        &RequireHandler::OnRequireFormulaAttack, this, std::placeholders::_1));

  return true;
}

void RequireHandler::Finalize() {
  REQUIRE_DISPATCHER()->Detach(require::REQUIRE_FORMULA_ATTACK);
}

#undef REQUIRE_DISPATCHER

void RequireHandler::SendEventCombatDeath(int32_t combat_id,
    const ::protocol::CombatTarget &target) {
  event::EventCombatDeath event;
  event.set_combat_id(combat_id);
  *event.mutable_target() = target;
  FORMULA_APP()->GetEventDispatcher()->Dispatch(event::EVENT_COMBAT_DEATH, &event);
}

int RequireHandler::FormulaDamage(CombatWarriorField *active,
    CombatWarriorField *passive) {
  ::protocol::CombatWarriorFields &active_fields = active->GetFields();
  ::protocol::CombatWarriorFields &passive_fields = passive->GetFields();

  int damage = active_fields.attack() - passive_fields.defence();
  if (damage <= 0) {
    damage = 1;
  }

  if (passive_fields.hp() > damage) {
    passive_fields.set_hp(passive_fields.hp() - damage);
  } else {
    passive_fields.set_hp(0);
  }

  return passive_fields.hp();
}

int RequireHandler::FormulaDamage(CombatWarriorField *active,
    CombatBuildingField *passive) {
  ::protocol::CombatWarriorFields &active_fields = active->GetFields();
  ::protocol::CombatBuildingFields &passive_fields = passive->GetFields();

  int damage = active_fields.attack();
  if (damage <= 0) {
    damage = 1;
  }

  if (passive_fields.hp() > damage) {
    passive_fields.set_hp(passive_fields.hp() - damage);
  } else {
    passive_fields.set_hp(0);
  }

  return passive_fields.hp();
}

int RequireHandler::OnRequireFormulaAttack(ProtoMessage *data) {
  require::RequireFormulaAttack *message = (require::RequireFormulaAttack *)data;

  CombatField *combat_field = CombatFieldManager::GetInstance()->Get(
      message->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("[FORMULA] CombatFieldManager::Get(%d) failed.",
        message->combat_id());
    return -1;
  }

  CombatWarriorField *combat_warrior_field = combat_field->GetWarrior(
      message->warrior_id());
  if (combat_warrior_field == NULL) {
    MYSYA_ERROR("[FORMULA] CombatField::GetWarrior(%d) failed.",
        message->warrior_id());
    return -1;
  }

  if (message->target().type() == ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    CombatWarriorField *target_combat_warrior_field = combat_field->GetWarrior(
        message->target().id());
    if (target_combat_warrior_field == NULL) {
      MYSYA_ERROR("[FORMULA] CombatField::GetWarrior(%d) failed.",
          message->target().id());
      return -1;
    }

    this->FormulaDamage(combat_warrior_field, target_combat_warrior_field);

    if (target_combat_warrior_field->GetFields().hp() <= 0) {
      this->SendEventCombatDeath(message->combat_id(), message->target());
    }

    return 0;
  } else if (message->target().type() == ::protocol::COMBAT_ENTITY_TYPE_BUILDING) {
    CombatBuildingField *target_combat_building_field = combat_field->GetBuilding(
        message->target().id());
    if (target_combat_building_field == NULL) {
      MYSYA_ERROR("[FORMULA] CombatField::GetBuilding(%d) failed.",
          message->target().id());
      return -1;
    }

    this->FormulaDamage(combat_warrior_field, target_combat_building_field);

    if (target_combat_building_field->GetFields().hp() <= 0) {
      this->SendEventCombatDeath(message->combat_id(), message->target());
    }

    return 0;
  } else {
    return -1;
  }

  return 0;
}

#undef FORMULA_APP

}  // namespace formula
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
