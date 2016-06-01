#include "tutorial/orcas/combat/server/scene/warrior.h"

#include <mysya/ioevent/logger.h>
#include <mysya/util/timestamp.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"
#include "tutorial/orcas/combat/server/scene/scene.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

Warrior::Warrior()
  : scene_(NULL), host_(NULL) {}
Warrior::~Warrior() {}

bool Warrior::Initialize(CombatWarriorField *host, Scene *scene) {
  this->host_ = host;
  this->scene_ = scene;

  if (this->move_action_.Initialize(this) == false) {
    MYSYA_ERROR("[SCENE] MoveAction::Initialize() failed.");
    return false;
  }

  return true;
}

void Warrior::Finalize() {
  this->move_action_.Finalize();
  this->host_ = NULL;
  this->scene_ = NULL;
}

int32_t Warrior::GetId() const {
  return this->host_->GetFields().id();
}

Scene *Warrior::GetScene() {
  return this->scene_;
}

CombatWarriorField *Warrior::GetHost() {
  return this->host_;
}

::protocol::Position Warrior::GetPos() {
  ::protocol::Position pos;
  pos.set_x(this->host_->GetFields().origin_pos_x());
  pos.set_y(this->host_->GetFields().origin_pos_y());

  return pos;
}

int32_t Warrior::GetMoveSpeed() const {
  return 1000 / this->host_->GetFields().move_speed();
}

MoveAction *Warrior::GetMoveAction() {
  return &this->move_action_;
}

void Warrior::DispatchMoveActionEvent(const ::protocol::Position &dest_pos,
    const PositionVector &paths) {
  event::EventCombatMoveAction event;
  event.set_combat_id(this->scene_->GetId());

  const ::mysya::util::Timestamp &begin_timestamp =
    this->GetHost()->GetRoleField()->GetCombatField()->GetBeginTimestamp();
  const ::mysya::util::Timestamp &now_timestamp =
    SceneApp::GetInstance()->GetHost()->GetTimestamp();

  ::protocol::CombatAction *action = event.mutable_action();
  action->set_type(::protocol::COMBAT_ACTION_TYPE_MOVE);
  action->set_timestamp(now_timestamp.DistanceSecond(begin_timestamp));

  ::protocol::CombatMoveAction *move_action = action->mutable_move_action();
  move_action->add_warrior_id(this->GetId());
  *move_action->mutable_pos() = dest_pos;
  for (PositionVector::const_iterator iter = paths.begin();
      iter != paths.end(); ++iter) {
    *move_action->add_paths() = *iter;
  }

  SceneApp::GetInstance()->GetEventDispatcher()->Dispatch(
      event::EVENT_COMBAT_MOVE_ACTION, &event);
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
