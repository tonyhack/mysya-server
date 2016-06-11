#include "tutorial/orcas/combat/server/ai/building.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

Building::Building()
  : host_(NULL), targeted_num_(0),
    timer_id_recovery_(-1), status_host_(this),
    status_retrieve_(this) {}

Building::~Building() {}

bool Building::Initialize(CombatBuildingField *host) {
  this->host_ = host;
  this->targeted_num_ = 0;
  this->present_status_ = &this->status_host_;

  this->present_status_->Start();

  return true;
}

void Building::Finalize() {
  this->present_status_->Stop();

  this->present_status_ = NULL;
  this->targeted_num_ = 0;
  this->host_ = NULL;
}

int32_t Building::GetId() const {
  return this->host_->GetId();
}

int32_t Building::GetCombatId() const {
  return this->host_->GetCombatField()->GetId();
}

Building::GlobalId Building::GetGlobalId() const {
  return GlobalId(this->GetCombatId(), this->GetId());
}

CombatBuildingField *Building::GetHost() {
  return this->host_;
}

int32_t Building::GetTargetedNum() const {
  return this->targeted_num_;
}

void Building::IncTargetedNum() {
  ++this->targeted_num_;
}

void Building::DecTargetedNum() {
  --this->targeted_num_;
}

BuildingStatus *Building::GetPresentStatus() {
  return this->present_status_;
}

static void SendEventCombatBuildingSwitchStatus(int32_t combat_id, int32_t building_id) {
  event::EventCombatBuildingSwitchStatus event;
  event.set_combat_id(combat_id);
  event.set_building_id(building_id);
  AiApp::GetInstance()->GetEventDispatcher()->Dispatch(
      event::EVENT_COMBAT_BUILDING_SWITCH_STATUS, &event);
}

bool Building::GotoStatus(int status) {
  BuildingStatus *goto_status = NULL;

  switch (status) {
    case ::protocol::BUILDING_STATUE_TYPE_HOST:
      goto_status = &this->status_host_;
      break;
    case ::protocol::BUILDING_STATUE_TYPE_RETRIEVE:
      goto_status = &this->status_retrieve_;
      break;
    default:
      break;
  }

  if (goto_status == NULL) {
    MYSYA_ERROR("[AI] status(%d) is invalid.", status);
    return false;
  }

  if (this->present_status_ != NULL) {
    MYSYA_ERROR("[AI] Building(%d,%d) status(%d->%d).", this->GetId(),
        this->GetCombatId(), this->present_status_->GetType(),
        goto_status->GetType());

    this->present_status_->Stop();
  }

  this->present_status_ = goto_status;
  this->present_status_->Start();
  this->host_->GetFields().set_status(this->present_status_->GetType());
  SendEventCombatBuildingSwitchStatus(this->GetCombatId(), this->GetId());

  return true;
}

void Building::RecoveryHp() {
  ::protocol::CombatBuildingFields &fields = this->host_->GetFields();
  fields.set_hp(fields.hp() + fields.hp_recovery());

  if (fields.hp() > fields.max_hp()) {
    fields.set_hp(fields.max_hp());
  }

  this->present_status_->OnRecoverHp();
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
