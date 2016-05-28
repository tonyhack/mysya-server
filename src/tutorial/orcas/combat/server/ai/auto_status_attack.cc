#include "tutorial/orcas/combat/server/ai/auto_status_attack.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusAttack::AutoStatusAttack()
  : AutoStatus(host), timer_id_attack_(0) {}
AutoStatusAttack::~AutoStatusAttack() {}

void AutoStatusAttack::Start() {
  this->SetAttackTimer();

  if (this->host_->AttackTarget() == false) {
    MYSYA_ERROR("[AI] Auto::AttackTarget() failed.");
    return;
  }
}

void AutoStatusAttack::Stop() {
  this->ResetAttackTimer();
}

void AutoStatusAttack::SetAttackTimer() {
  CombatWarriorField *combat_warrior_field = this->host_->GetHost();
  if (combat_warrior_field == NULL) {
    MYSYA_ERROR("[AI] Auto::GetHost() failed.");
    return;
  }

  const ::protocol::CombatWarriorFields &fields = combat_warrior_field->GetFields();
  int attack_interval_ms = fields.attack_speed();

  ::protocol::WarriorDescription *warrior_description =
    combat_warrior_field->GetDescription();
  if (warrior_description == NULL) {
    MYSYA_ERROR("[AI] CombatWarriorField::GetDescription() failed.");
    return;
  }

  this->timer_id_attack_ = AiApp::GetInstance()->GetHost()->StartTimer(
      attack_interval_ms, std::bind(&AutoStatusAttack::OnTimerAttack,
        std::placehoder::_1));
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

  const ::protocol::CombatTarget &target = this->GetTarget();
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
