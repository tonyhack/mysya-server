#include "tutorial/orcas/combat/server/ai/auto_status_chase.h"

#include <functional>

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/auto.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_scene.pb.h"
#include "tutorial/orcas/protocol/cc/combat.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusChase::AutoStatusChase(Auto *host)
  : AutoStatus(host), refind_path_(false),
  timer_id_refind_path_(-1) {
  this->AttachEvent(event::EVENT_SCENE_MOVE_STEP, std::bind(
        &AutoStatusChase::OnEventSceneMoveStep, this, std::placeholders::_1));
  this->AttachEvent(event::EVENT_COMBAT_DEATH, std::bind(
        &AutoStatusChase::OnEventCombatDeath, this, std::placeholders::_1));
  this->AttachEvent(event::EVENT_COMBAT_CONVERT_CAMP, std::bind(
        &AutoStatusChase::OnEventCombatConvertCamp, this, std::placeholders::_1));
}

AutoStatusChase::~AutoStatusChase() {
  this->DetachEvent(event::EVENT_SCENE_MOVE_STEP);
  this->DetachEvent(event::EVENT_COMBAT_DEATH);
  this->DetachEvent(event::EVENT_COMBAT_CONVERT_CAMP);
}

void AutoStatusChase::Start() {
  if (this->CheckTargetAttackReachable() == true) {
    this->GotoStatus(AutoStatus::ATTACK);
    return;
  }

  if (this->host_->MoveTarget() == false) {
    this->GotoStatus(AutoStatus::SEARCH);
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

AutoStatus::type AutoStatusChase::GetType() const {
  return AutoStatus::CHASE;
}

void AutoStatusChase::SetRefindPathTimer() {
  this->timer_id_refind_path_ = AiApp::GetInstance()->GetHost()->StartTimer(
      AutoStatusChase::kRefindPathMsec_, std::bind(&AutoStatusChase::OnTimerRefindPath,
        this, std::placeholders::_1), 1);
  this->refind_path_ = true;
}

bool AutoStatusChase::CheckTargetAttackReachable() {
  const ::protocol::WarriorDescription *warrior_description =
    this->host_->GetHost()->GetDescription();
  if (warrior_description == NULL) {
    MYSYA_ERROR("[AI] CombatWarriorField::GetDescription() failed.");
    return false;
  }

  int target_distance = this->host_->GetTargetDistance();
  if (target_distance < 0) {
    MYSYA_ERROR("[AI] Auto::GetTargetDistance() failed.");
    return false;
  }

  // TODO: if target_distance > search_range giveup target, and goto search status.
  return target_distance <= warrior_description->attack_range();
}

void AutoStatusChase::OnTimerRefindPath(int64_t timer_id) {
  this->timer_id_refind_path_ = -1;
  this->refind_path_ = false;

  if (this->CheckTargetAttackReachable() == true) {
    this->GotoStatus(AutoStatus::ATTACK);
    return;
  }

  if (this->host_->MoveTarget() == false) {
    this->GotoStatus(AutoStatus::SEARCH);
    return;
  }
}

void AutoStatusChase::OnEventSceneMoveStep(const ProtoMessage *data) {
  event::EventSceneMoveStep *event = (event::EventSceneMoveStep *)data;

  const ::protocol::CombatEntity &target = this->host_->GetTarget();
  if (this->host_->GetId() != event->warrior_id() &&
      target.id() != event->warrior_id()) {
    return;
  }

  if (this->CheckTargetAttackReachable() == true) {
    this->GotoStatus(AutoStatus::ATTACK);
    return;
  }

  if (target.id() == event->warrior_id() && this->refind_path_ == false) {
    this->SetRefindPathTimer();
  }
}

void AutoStatusChase::OnEventCombatDeath(const ProtoMessage *data) {
  const event::EventCombatDeath *event = (const event::EventCombatDeath *)data;

  const ::protocol::CombatEntity &target = this->host_->GetTarget();
  if (target.id() != event->target().id() || target.type() != event->target().type()) {
    return;
  }

  this->GotoStatus(AutoStatus::SEARCH);
}

void AutoStatusChase::OnEventCombatConvertCamp(const ProtoMessage *data) {
  const event::EventCombatConvertCamp *event = (const event::EventCombatConvertCamp *)data;

  const ::protocol::CombatEntity &target = this->host_->GetTarget();
  if (target.id() != event->host().id() || target.type() != event->host().type()) {
    return;
  }

  this->GotoStatus(AutoStatus::SEARCH);
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
