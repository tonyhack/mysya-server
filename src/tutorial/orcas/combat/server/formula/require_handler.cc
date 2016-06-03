#include "tutorial/orcas/combat/server/formula/require_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/combat_warrior_field_pool.h"
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
    int32_t opponent_warrior_id, const ::protocol::CombatEntity &target) {
  event::EventCombatDeath event;
  event.set_combat_id(combat_id);
  *event.mutable_target() = target;
  event.set_opponent_warrior_id(opponent_warrior_id);
  FORMULA_APP()->GetEventDispatcher()->Dispatch(event::EVENT_COMBAT_DEATH, &event);
}

void RequireHandler::SendEventCombatConvertCamp(int32_t combat_id,
    int32_t original_camp_id, int32_t original_host_id, int32_t camp_id,
    int32_t host_id, const ::protocol::CombatEntity &host) {
  event::EventCombatConvertCamp event;
  event.set_combat_id(combat_id);
  *event.mutable_host() = host;
  event.set_camp_id(camp_id);
  event.set_host_id(host_id);
  event.set_original_camp_id(camp_id);
  event.set_original_host_id(host_id);
  FORMULA_APP()->GetEventDispatcher()->Dispatch(event::EVENT_COMBAT_CONVERT_CAMP, &event);
}

void RequireHandler::SendEventCombatAttack(int32_t combat_id, int32_t warrior_id,
    const ::protocol::CombatEntity &target, int32_t damage) {
  event::EventCombatAttack event1;
  event1.set_combat_id(combat_id);
  event1.set_warrior_id(warrior_id);
  *event1.mutable_target() = target;
  event1.set_damage(damage);
  FORMULA_APP()->GetEventDispatcher()->Dispatch(event::EVENT_COMBAT_ATTACK, &event1);

  event::EventCombatAttacked event2;
  event2.set_combat_id(combat_id);
  event2.set_warrior_id(warrior_id);
  *event2.mutable_host() = target;
  event2.set_damage(damage);
  FORMULA_APP()->GetEventDispatcher()->Dispatch(event::EVENT_COMBAT_ATTACKED, &event2);
}

int RequireHandler::FormulaWarriorDamage(CombatWarriorField *active,
    CombatWarriorField *passive) {
  ::protocol::CombatWarriorFields &active_fields = active->GetFields();
  ::protocol::CombatWarriorFields &passive_fields = passive->GetFields();

  int damage = active_fields.attack() - passive_fields.defence();
  if (damage < 1) {
    damage = 1;
  }

  if (passive_fields.hp() > damage) {
    passive_fields.set_hp(passive_fields.hp() - damage);
  } else {
    passive_fields.set_hp(0);
  }

  return passive_fields.hp();
}

int RequireHandler::FormulaBuildingDamage(CombatWarriorField *active,
    CombatBuildingField *passive) {
  ::protocol::CombatWarriorFields &active_fields = active->GetFields();
  ::protocol::CombatBuildingFields &passive_fields = passive->GetFields();

  int damage = active_fields.attack();
  if (damage < 1) {
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

  int damage = 0;

  if (message->target().type() == ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    CombatWarriorField *target_combat_warrior_field = combat_field->GetWarrior(
        message->target().id());
    if (target_combat_warrior_field == NULL) {
      MYSYA_ERROR("[FORMULA] CombatField::GetWarrior(%d) failed.",
          message->target().id());
      return -1;
    }

    damage = this->FormulaWarriorDamage(combat_warrior_field, target_combat_warrior_field);
    if (damage < 0) {
      MYSYA_DEBUG("[FORMULA] FormulaWarriorDamage() failed.");
      return -1;
    }

    this->SendEventCombatAttack(message->combat_id(), message->warrior_id(),
        message->target(), damage);

    if (target_combat_warrior_field->GetFields().hp() <= 0) {
      MYSYA_DEBUG("[FORMULA] warrior(%d) dead.", target_combat_warrior_field->GetId());
      this->SendEventCombatDeath(message->combat_id(), message->warrior_id(),
          message->target());

      combat_field->RemoveWarrior( message->target().id());
      target_combat_warrior_field->Finalize();
      CombatWarriorFieldPool::GetInstance()->Deallocate(target_combat_warrior_field);
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

    damage = this->FormulaBuildingDamage(combat_warrior_field, target_combat_building_field);
    if (damage < 0) {
      MYSYA_DEBUG("[FORMULA] FormulaBuildingDamage() failed.");
      return -1;
    }

    this->SendEventCombatAttack(message->combat_id(), message->warrior_id(),
        message->target(), damage);

    if (target_combat_building_field->GetFields().hp() <= 0) {
      // original id.
      int32_t original_camp_id = target_combat_building_field->GetFields().camp_id();
      int32_t original_host_id = target_combat_building_field->GetFields().host_id();

      // convert building's camp_id.
      target_combat_building_field->GetFields().set_camp_id(
          combat_warrior_field->GetFields().camp_id());
      target_combat_building_field->GetFields().set_host_id(
          combat_warrior_field->GetFields().host_id());

      // send event.
      this->SendEventCombatConvertCamp(message->combat_id(), original_camp_id,
          original_host_id, target_combat_building_field->GetFields().camp_id(),
          combat_warrior_field->GetFields().host_id(), message->target());

      // TODO: check if can settlement?
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
