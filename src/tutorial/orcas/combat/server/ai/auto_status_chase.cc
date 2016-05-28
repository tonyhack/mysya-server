#include "tutorial/orcas/combat/server/ai/auto_status_chase.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusChase::AutoStatusChase(Auto *host)
  : AutoStatus(host), refind_path_(false),
    timer_id_refind_path_(-1) {
  this->AttachEvent(event::EVENT_SCENE_MOVE_STEP, std::bind(
        &AutoStatusChase::OnEventSceneMoveStep, this, std::placehoder::_1));
}

AutoStatusChase::~AutoStatusChase() {
  this->AttachEvent(event::EVENT_SCENE_MOVE_STEP);
}

void AutoStatusChase::Start() {
  if (this->MoveTarget() == false) {
    this->host_->GotoStatus(AutoStatus::SEARCH);
    return;
  }
}

void AutoStatusChase::Stop() {
  if (this->timer_id_refind_path_ != -1) {
    AiApp::GetInstance()->GetHost()->StopTimer(this->timer_id_refind_path_);
    this->timer_id_refind_path_ = -1;
    this->refind_path_ = false;
  }
}

void AutoStatusChase::SetRefindPathTimer() {
  this->timer_id_refind_path_ = AiApp::GetInstance()->GetHost()->StartTimer(
      AutoStatusChase::kRefindPathMsec_, std::bind(&AutoStatusChase::OnTimerRefindPath,
        std::placehoder::_1), 1);
  this->refind_path_ = true;
}

void AutoStatusChase::OnTimerRefindPath(int64_t timer_id) {
  this->refind_path_ = false;
  if (this->host_->MoveTarget() == false) {
    this->SetRefindPathTimer();
  }
}

void AutoStatusChase::OnEventSceneMoveStep(const ProtoMessage *data) {
  event::EventSceneMoveStep *event = (event::EventSceneMoveStep *)data;

  CombatField *combat_field = this->host_->GetHost()->GetRoleField()->GetCombatField();
  if (combat_field == NULL) {
    MYSYA_ERROR("[AI] CombatRoleField::GetCombatField() failed.");
    return;
  }

  do {
    const ::protocol::CombatTarget &target = this->host_->GetTarget();

    // the moving warrior is myself.
    if (this->host_->GetId() == event->warrior_id()) {
      ::protocol::WarriorDescription *warrior_description =
        this->host_->GetHost()->GetDescription();
      if (warrior_description == NULL) {
        MYSYA_ERROR("[AI] CombatWarriorField::GetDescription() failed.");
        return;
      }

      int target_distance = this->host_->GetTargetDistance();
      if (target_distance < 0) {
        MYSYA_ERROR("[AI] Auto::GetTargetDistance() failed.");
        return;
      }

      // TODO: if target_distance > search_range giveup target, and goto search status.
      if (target_distance <= warrior_description->attack_range()) {
        this->GotoStatus(AutoStatus::ATTACK);
        return;
      }

      if (this->refind_path_ == false) {
        this->SetRefindPathTimer();
      }

      return;
    }

    // the moving warrior is my target.
    if (target.id() == event->warrior_id()) {
      CombatWarriorField *target_combat_warrior_field = combat_field->GetWarrior(
          event->warrior_id());
      if (target_combat_warrior_field == NULL) {
        MYSYA_ERROR("[AI] CombatField::GetWarrior(%d) failed.",
            event->warrior_id());
        return;
      }
    }
  } while (false);
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
