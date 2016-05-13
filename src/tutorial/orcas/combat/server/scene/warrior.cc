#include "tutorial/orcas/combat/server/scene/warrior.h"

#include <mysya/util/timestamp.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
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

  this->move_action_.SetHost(this);

  return true;
}

void Warrior::Finalize() {
  this->host_ = NULL;
  this->scene_ = NULL;
  this->move_action_.SetHost(NULL);
}

int32_t Warrior::GetId() const {
  return this->host_->GetFields().id();
}

::protocol::Position Warrior::GetPos() {
  ::protocol::Position pos;
  pos.set_x(this->host_->GetFields().origin_pos_x());
  pos.set_y(this->host_->GetFields().origin_pos_y());

  return pos;
}

int32_t Warrior::GetMoveSpeed() const {
  return this->host_->GetFields().move_speed();
}

MoveAction *Warrior::GetMoveAction() {
  return &this->move_action_;
}

void Warrior::SyncMoveAction(const ::protocol::Position &dest_pos) {
  const ::mysya::util::Timestamp &begin_timestamp =
    this->host_->GetRoleField()->GetCombatField()->GetBeginTimestamp();
  const ::mysya::util::Timestamp &now_timestamp =
    SceneApp::GetInstance()->GetHost()->GetTimestamp();

  ::protocol::MessageCombatActionSync message;

  ::protocol::CombatAction *action = message.mutable_action();
  action->set_type(::protocol::COMBAT_ACTION_TYPE_MOVE);
  action->set_timestamp(now_timestamp.DistanceSecond(begin_timestamp));

  ::protocol::CombatMoveAction *move_action = action->mutable_move_action();
  move_action->add_warrior_id(this->GetId());
  *move_action->mutable_pos() = dest_pos;

  // TODO: 是否应该发事件，处理发送、处理战斗记录?
  this->GetHost()->GetRoleField()->GetCombatField()->BroadcastMessage(
      ::protocol::MESSAGE_COMBAT_ACTION_SYNC, message);
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
