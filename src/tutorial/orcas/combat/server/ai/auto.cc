#include "tutorial/orcas/combat/server/ai/auto.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/math.h"
#include "tutorial/orcas/combat/server/require_dispatcher.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/event_observer.h"
#include "tutorial/orcas/combat/server/require/cc/require.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_formula.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_scene.pb.h"
#include "tutorial/orcas/protocol/cc/position.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

Auto::Auto()
  : host_(NULL),
    present_status_(NULL),
    status_attack_(this),
    status_chase_(this),
    status_search_(this) {}
Auto::~Auto() {}

bool Auto::Initialize(CombatWarriorField *host) {
  this->host_ = host;
  this->present_status_ = &this->status_search_;
  this->target_.Clear();

  this->present_status_->Start();

  return true;
}

void Auto::Finalize() {
  this->present_status_->Stop();
  this->present_status_ = NULL;

  this->ResetTarget();
  this->host_ = NULL;
}

int32_t Auto::GetId() const {
  return this->host_->GetId();
}

int32_t Auto::GetCombatId() const {
  return this->host_->GetCombatField()->GetId();
}

Auto::AutoGlobalId Auto::GetGlobalId() const {
  return AutoGlobalId(this->GetCombatId(), this->GetId());
}

CombatWarriorField *Auto::GetHost() {
  return this->host_;
}

void Auto::SetTarget(::protocol::CombatEntityType type, int32_t id) {
  this->target_.set_type(type);
  this->target_.set_id(id);

  EventObserver::GetInstance()->Add(this->GetCombatId(),
      this->target_.id(), this->GetId());
}

void Auto::ResetTarget() {
  if (this->target_.id() != 0) {
    EventObserver::GetInstance()->Remove(this->GetCombatId(),
        this->target_.id(), this->GetId());
    this->target_.set_id(0);
  }
}

::protocol::CombatEntity &Auto::GetTarget() {
  return this->target_;
}

// TODO: this is a straight-line distance now, but this should be a-star distance.
int Auto::GetTargetDistance() const {
  if (this->target_.id() == 0) {
    return -1;
  }

  CombatField *combat_field = this->host_->GetRoleField()->GetCombatField();
  if (combat_field == NULL) {
    return -1;
  }

  ::protocol::Position target_pos;

  if (this->target_.type() == ::protocol::COMBAT_ENTITY_TYPE_BUILDING) {
    CombatBuildingField *combat_building_field = combat_field->GetBuilding(
        this->target_.id());
    if (combat_building_field == NULL) {
      return -1;
    }

    target_pos.set_x(combat_building_field->GetFields().pos_x());
    target_pos.set_y(combat_building_field->GetFields().pos_y());
  } else if (this->target_.type() == ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    CombatWarriorField *combat_warrior_field = combat_field->GetWarrior(
        this->target_.id());
    if (combat_warrior_field == NULL) {
      return -1;
    }

    target_pos.set_x(combat_warrior_field->GetFields().origin_pos_x());
    target_pos.set_y(combat_warrior_field->GetFields().origin_pos_y());
  } else {
    return -1;
  }

  return math::distance(target_pos.x(), target_pos.y(),
      this->host_->GetFields().origin_pos_x(), this->host_->GetFields().origin_pos_y());
}

AutoStatus *Auto::GetPresentStatus() {
  return this->present_status_;
}

bool Auto::GotoStatus(int status) {
  AutoStatus *goto_status = NULL;

  switch (status) {
    case AutoStatus::SEARCH:
      goto_status = &this->status_search_;
      break;
    case AutoStatus::CHASE:
      goto_status = &this->status_chase_;
      break;
    case AutoStatus::ATTACK:
      goto_status = &this->status_attack_;
      break;
    default:
      break;
  }

  if (goto_status == NULL) {
    MYSYA_ERROR("[AI] status(%d) is invalid.", status);
    return false;
  }

  if (this->present_status_ != NULL) {
    MYSYA_DEBUG("[AI] Auto(%d,%d) status(%d->%d).", this->GetId(),
        this->GetCombatId(), this->present_status_->GetType(),
        goto_status->GetType());

    this->present_status_->Stop();
    EventObserver::GetInstance()->Add(this->GetCombatId(),
        this->GetId(), this->GetId());
  }

  this->present_status_ = goto_status;
  this->present_status_->Start();
  EventObserver::GetInstance()->Add(this->GetCombatId(),
      this->GetId(), this->GetId());

  return true;
}

bool Auto::MoveTarget() {
  MYSYA_DEBUG("[AI] Auto(%d,%d) Move taget.", this->GetId(), this->GetCombatId());

  if (this->target_.id() == 0) {
    return false;
  }

  CombatField *combat_field = this->host_->GetRoleField()->GetCombatField();
  if (combat_field == NULL) {
    MYSYA_ERROR("[AI] CombatRoleField::GetCombatField() failed.");
    return false;
  }

  ::protocol::Position pos;
  if (this->target_.type() == ::protocol::COMBAT_ENTITY_TYPE_BUILDING) {
    CombatBuildingField *combat_building_field = combat_field->GetBuilding(
        this->target_.id());
    if (combat_building_field == NULL) {
      MYSYA_ERROR("[AI] CombatField::GetBuilding(%d) failed.", this->target_.id());
      return false;
    }
    pos.set_x(combat_building_field->GetFields().pos_x());
    pos.set_y(combat_building_field->GetFields().pos_y());
  } else if (this->target_.type() == ::protocol::COMBAT_ENTITY_TYPE_WARRIOR) {
    CombatWarriorField *combat_warrior_field = combat_field->GetWarrior(
        this->target_.id());
    if (combat_warrior_field == NULL) {
      MYSYA_ERROR("[AI] CombatField::GetWarrior(%d) failed.", this->target_.id());
      return false;
    }
    pos.set_x(combat_warrior_field->GetFields().origin_pos_x());
    pos.set_y(combat_warrior_field->GetFields().origin_pos_y());
  } else {
    MYSYA_ERROR("[AI] CombatWarriorField::target_.type(%d) error.",
        this->target_.type());
    return false;
  }

  require::RequireSceneMove require_message;
  require_message.set_combat_id(combat_field->GetId());
  require_message.set_warrior_id(this->host_->GetId());
  *require_message.mutable_dest_pos() = pos;
  if (AiApp::GetInstance()->GetRequireDispatcher()->Dispatch(
        require::REQUIRE_SCENE_MOVE, &require_message) < 0) {
    MYSYA_ERROR("[AI] REQUIRE_SCENE_MOVE failed.");
    return false;
  }

  return true;
}

bool Auto::SearchTarget() {
  CombatRoleField *combat_role_field = this->host_->GetRoleField();
  if (combat_role_field == NULL) {
    MYSYA_ERROR("[AI] CombatWarriorField::GetRoleField() failed.");
    return false;
  }

  CombatField *combat_field = combat_role_field->GetCombatField();
  if (combat_field == NULL) {
    MYSYA_ERROR("[AI] CombatRoleField::GetCombatField() failed.");
    return false;
  }

  const ::protocol::WarriorDescription *warrior_description =
    combat_role_field->GetWarriorDescription(this->host_->GetFields().conf_id());
  if (warrior_description == NULL) {
    MYSYA_ERROR("[AI] CombatRoleField::GetWarriorDescription(%d) failed.",
        this->host_->GetFields().conf_id());
    return false;
  }

  require::RequireSceneFetch require_message;
  require_message.set_combat_id(combat_field->GetId());
  require_message.set_except_camp_id(this->host_->GetFields().camp_id());
  require_message.mutable_pos()->set_x(this->host_->GetFields().origin_pos_x());
  require_message.mutable_pos()->set_y(this->host_->GetFields().origin_pos_y());
  // TODO: search range
  require_message.set_range(4);
  if (AiApp::GetInstance()->GetRequireDispatcher()->Dispatch(
        require::REQUIRE_SCENE_FETCH, &require_message) < 0) {
    MYSYA_ERROR("[AI] Dispatch REQUIRE_SCENE_FETCH failed.");
    return false;
  }

  for (int i = 0; i < require_message.building_size(); ++i) {
    int32_t building_id = require_message.building(i);
    CombatBuildingField *combat_building_field = combat_field->GetBuilding(
        building_id);
    if (combat_building_field == NULL) {
      continue;
    }

    this->SetTarget(::protocol::COMBAT_ENTITY_TYPE_BUILDING, building_id);
    return true;
  }

  for (int i = 0; i < require_message.warrior_size(); ++i) {
    int32_t warrior_id = require_message.warrior(i);
    CombatWarriorField *combat_warrior_field = combat_field->GetWarrior(
        warrior_id);
    if (combat_warrior_field == NULL) {
      continue;
    }

    this->SetTarget(::protocol::COMBAT_ENTITY_TYPE_WARRIOR, warrior_id);
    return true;
  }

  return false;
}

bool Auto::AttackTarget() {
  CombatRoleField *combat_role_field = this->host_->GetRoleField();
  if (combat_role_field == NULL) {
    MYSYA_ERROR("[AI] CombatWarriorField::GetRoleField() failed.");
    return false;
  }

  CombatField *combat_field = combat_role_field->GetCombatField();
  if (combat_field == NULL) {
    MYSYA_ERROR("[AI] CombatRoleField::GetCombatField() failed.");
    return false;
  }

  require::RequireFormulaAttack require_message;
  require_message.set_combat_id(combat_field->GetId());
  require_message.set_warrior_id(this->GetId());
  *require_message.mutable_target() = this->GetTarget();
  if (AiApp::GetInstance()->GetRequireDispatcher()->Dispatch(
        require::REQUIRE_FORMULA_ATTACK, &require_message) < 0) {
    MYSYA_ERROR("[AI] REQUIRE_FORMULA_ATTACK failed.");
    return false;
  }

  MYSYA_DEBUG("[AI] Attack {warrior[camp(%d),id(%d)]} => {target[type(%d),id(%d)]}.",
      this->host_->GetFields().camp_id(), this->GetId(),
      this->target_.type(), this->target_.id());

  return true;
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
