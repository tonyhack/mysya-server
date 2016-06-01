#include "tutorial/orcas/combat/server/ai/auto_status_attack.h"

#include <functional>

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/auto.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"
#include "tutorial/orcas/protocol/cc/warrior.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusAttack::AutoStatusAttack(Auto *host)
  : AutoStatus(host), timer_id_attack_(0) {
  this->AttachEvent(event::EVENT_COMBAT_DEATH, std::bind(
        &AutoStatusAttack::OnEventCombatDeath, this, std::placeholders::_1));
}

AutoStatusAttack::~AutoStatusAttack() {
  this->DetachEvent(event::EVENT_COMBAT_DEATH);
}

void AutoStatusAttack::Start() {
  this->SetAttackTimer();

  // TODO: break move.

  event::EventCombatLockTarget event;
  event.set_combat_id(this->host_->GetHost()->GetCombatField()->GetId());
  event.set_warrior_id(this->host_->GetId());
  *event.mutable_target() = this->host_->GetTarget();
  AiApp::GetInstance()->GetEventDispatcher()->Dispatch(event::EVENT_COMBAT_LOCK_TARGET, &event);

  if (this->host_->AttackTarget() == false) {
    MYSYA_ERROR("[AI] Auto::AttackTarget() failed.");
    this->GotoStatus(AutoStatus::SEARCH);
    return;
  }
}

void AutoStatusAttack::Stop() {
  this->ResetAttackTimer();
}

AutoStatus::type AutoStatusAttack::GetType() const {
  return AutoStatus::ATTACK;
}

void AutoStatusAttack::SetAttackTimer() {
  CombatWarriorField *combat_warrior_field = this->host_->GetHost();
  if (combat_warrior_field == NULL) {
    MYSYA_ERROR("[AI] Auto::GetHost() failed.");
    return;
  }

  const ::protocol::CombatWarriorFields &fields = combat_warrior_field->GetFields();
  int attack_interval_ms = fields.attack_speed();

  const ::protocol::WarriorDescription *warrior_description =
    combat_warrior_field->GetDescription();
  if (warrior_description == NULL) {
    MYSYA_ERROR("[AI] CombatWarriorField::GetDescription() failed.");
    return;
  }

  this->timer_id_attack_ = AiApp::GetInstance()->GetHost()->StartTimer(
      attack_interval_ms, std::bind(&AutoStatusAttack::OnTimerAttack,
        this, std::placeholders::_1));
}

void AutoStatusAttack::ResetAttackTimer() {
  if (this->timer_id_attack_ != -1) {
    AiApp::GetInstance()->GetHost()->StopTimer(this->timer_id_attack_);
    this->timer_id_attack_ = -1;
  }
}

void AutoStatusAttack::OnTimerAttack(int64_t timer_id) {
  this->host_->AttackTarget();
}

void AutoStatusAttack::OnEventCombatDeath(const ProtoMessage *data) {
  event::EventCombatDeath *event = (event::EventCombatDeath *)data;

  const ::protocol::CombatEntity &target = this->host_->GetTarget();
  if (target.id() != event->target().id() || target.type() != event->target().type()) {
    return;
  }

  this->GotoStatus(AutoStatus::SEARCH);
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
