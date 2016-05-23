#include "tutorial/orcas/combat/server/scene/move_action.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/scene/grid.h"
#include "tutorial/orcas/combat/server/scene/scene.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"
#include "tutorial/orcas/combat/server/scene/warrior.h"
#include "tutorial/orcas/protocol/cc/position.pb.h"
#include "tutorial/orcas/protocol/cc/warrior.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

MoveAction::MoveAction()
  : path_index_(0), host_(NULL),
    move_ms_(0), timer_move_token_(-1) {}
MoveAction::~MoveAction() {
  this->Reset();
}

void MoveAction::SetHost(Warrior *warrior) {
  this->host_ = warrior;
  this->move_ms_ = warrior->GetMoveSpeed();
}

void MoveAction::Start(const ::protocol::Position &dest_pos) {
  if (this->host_->GetPos().x() == dest_pos.x() &&
      this->host_->GetPos().y() == dest_pos.y()) {
    return;
  }

  Scene *scene = this->host_->GetScene();
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] Warrior::GetScene() failed.");
    return;
  }

  if (scene->GetWalkable(dest_pos) == false) {
    MYSYA_ERROR("[SCENE] Scene[%d]::GetWalkable(%d, %d) failed.",
        scene->GetMapId(), dest_pos.x(), dest_pos.y());
    return;
  }

  if (this->move_paths_.empty() == false) {
    this->Finish(false);
  }

  scene->SearchPath(this->host_->GetPos(), dest_pos, this->move_paths_);

  if (this->move_paths_.empty() == true) {
    MYSYA_ERROR("[SCENE] Scene[%d]::SearchPath() failed.",
        scene->GetMapId());
    return;
  }

  this->timer_move_token_ = SceneApp::GetInstance()->GetHost()->StartTimer(
      this->move_ms_, std::bind(&MoveAction::OnMoveTimer, this,
        std::placeholders::_1));

  this->MoveStep();
  this->host_->DispatchMoveActionEvent(dest_pos, this->move_paths_);
}

void MoveAction::Reset() {
  if (this->timer_move_token_ != -1) {
    SceneApp::GetInstance()->GetHost()->StopTimer(this->timer_move_token_);
    this->timer_move_token_ = -1;
  }

  this->path_index_ = 0;
  this->move_paths_.clear();
}

void MoveAction::Finish(bool success) {
  this->Reset();

  if (success == false) {
    this->host_->DispatchMoveActionEvent(this->host_->GetPos(), this->move_paths_);
  }
}

void MoveAction::OnMoveTimer(int64_t timer_id) {
  this->MoveStep();
}

void MoveAction::MoveStep() {
  Scene *scene = this->host_->GetScene();
  if (scene == NULL) {
    MYSYA_ERROR("[SCENE] Warrior::GetScene() failed.");
    return this->Finish(false);
  }

  ::protocol::CombatWarriorFields &fields = this->host_->GetHost()->GetFields();

  ::protocol::Position current_pos;
  current_pos.set_x(fields.origin_pos_x());
  current_pos.set_y(fields.origin_pos_y());

  Grid *current_grid = scene->GetGrid(current_pos);
  if (current_grid == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetGrid() failed.");
    return this->Finish(false);
  }

  const ::protocol::Position &next_pos(this->move_paths_[this->path_index_++]);

  Grid *next_grid = scene->GetGrid(next_pos);
  if (next_grid == NULL) {
    MYSYA_ERROR("[SCENE] Scene::GetGrid() failed.");
    return this->Finish(false);
  }

  current_grid->RemoveWarrior(this->host_);
  next_grid->AddWarrior(this->host_);

  fields.set_origin_pos_x(next_pos.x());
  fields.set_origin_pos_y(next_pos.y());

  if (this->path_index_ >= (int)this->move_paths_.size()) {
    this->Finish();
  }

  // TODO: for debug.
  this->host_->GetScene()->PrintStatusImage();
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
