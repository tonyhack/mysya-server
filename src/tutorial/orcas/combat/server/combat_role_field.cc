#include "tutorial/orcas/combat/server/combat_role_field.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/app_session.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/combat_warrior_field_pool.h"
#include "tutorial/orcas/combat/server/require/cc/require.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_scene.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

CombatRoleField::CombatRoleField()
  : argent_id_(0), camp_id_(0),
    building_num_(0), combat_field_(NULL),
    app_session_(NULL), app_server_(NULL) {}

CombatRoleField::~CombatRoleField() {}

bool CombatRoleField::Initialize(uint64_t argent_id, const std::string &name,
    AppServer *app_server) {
  this->SetArgentId(argent_id);
  this->name_ = name;
  this->app_server_ = app_server;
  this->app_session_ = NULL;
  this->building_num_ = 0;

  return true;
}

void CombatRoleField::Finalize() {
  if (this->app_session_ != NULL) {
    this->app_session_->Remove(this);
  }

  this->warrior_descriptions_.clear();

  this->SetArgentId(0);
  this->SetCampId(0);
  this->SetCombatField(NULL);
}

AppServer *CombatRoleField::GetAppServer() {
  return this->app_server_;
}

uint64_t CombatRoleField::GetArgentId() const {
  return this->argent_id_;
}

void CombatRoleField::SetArgentId(uint64_t value) {
  this->argent_id_ = value;
}

int32_t CombatRoleField::GetCampId() const {
  return this->camp_id_;
}

void CombatRoleField::SetCampId(int32_t value) {
  this->camp_id_ = value;
}

int32_t CombatRoleField::GetBuildingNum() const {
  return this->building_num_;
}

void CombatRoleField::SetBuildingNum(int32_t value) {
  this->building_num_ = value;
}

const std::string &CombatRoleField::GetName() const {
  return this->name_;
}

CombatField *CombatRoleField::GetCombatField() {
  return this->combat_field_;
}

void CombatRoleField::SetCombatField(CombatField *value) {
  this->combat_field_ = value;
}


void CombatRoleField::AddWarriorDescription(
    const ::protocol::WarriorDescription &warrior) {
  this->warrior_descriptions_.insert(std::make_pair(warrior.id(), warrior));
}

const ::protocol::WarriorDescription *CombatRoleField::GetWarriorDescription(
    int32_t id) const {
  const ::protocol::WarriorDescription *warrior = NULL;

  WarriorDescriptionMap::const_iterator iter = this->warrior_descriptions_.find(id);
  if (iter != this->warrior_descriptions_.end()) {
    warrior = &iter->second;
 }

  return warrior;
}

void CombatRoleField::SetAppSession(AppSession *session) {
  this->app_session_ = session;
  this->app_session_->Add(this);
}

void CombatRoleField::ResetAppSession() {
  this->app_session_ = NULL;
}

int CombatRoleField::SendMessage(const ::google::protobuf::Message &message) {
  if (this->app_session_ == NULL) {
    return -1;
  }

  return this->app_session_->SendMessage(message);
}

bool CombatRoleField::DoAction(const ::protocol::CombatAction &action) {
  bool result = false;

  switch (action.type()) {
    case ::protocol::COMBAT_ACTION_TYPE_BUILD:
      result = this->DoBuildAction(action.build_action());
      break;
    case ::protocol::COMBAT_ACTION_TYPE_MOVE:
      result = this->DoMoveAction(action.move_action());
      break;
    case ::protocol::COMBAT_ACTION_TYPE_LOCK_TARGET:
      result = this->DoLockTargetAction(action.lock_target_action());
      break;
    default:
      break;
  }

  return result;
}

bool CombatRoleField::DoBuildAction(const ::protocol::CombatBuildAction &action) {
  const ::protocol::WarriorDescription *warrior_description =
    this->GetWarriorDescription(action.warrior_conf_id());
  if (warrior_description == NULL) {
    MYSYA_ERROR("CombatRoleField::GetWarriorDescription(%d) failed.",
        action.warrior_conf_id());
    return false;
  }

  // TODO: 判断 action.building_id 是否可以建 action.warrior_conf_id
  CombatBuildingField *combat_building_field =
    this->combat_field_->GetBuilding(action.building_id());
  if (combat_building_field == NULL) {
    MYSYA_ERROR("CombatField::GetBuilding(%d) failed.",
        action.building_id());
    return false;
  }

  if (combat_building_field->GetFields().host_id() != this->GetArgentId()) {
    MYSYA_ERROR("CombatBuildingField.host_id(%ld) not matching(%ld).",
        combat_building_field->GetFields().host_id(), this->GetArgentId());
    return false;
  }

  CombatWarriorField *combat_warrior_field =
    CombatWarriorFieldPool::GetInstance()->Allocate();
  if (combat_warrior_field == NULL) {
    MYSYA_ERROR("CombatWarriorFieldPool::Allocate() failed.");
    return false;
  }

  if (combat_warrior_field->Initialize(this->combat_field_->AllocateId(), this,
        *warrior_description) == false) {
    MYSYA_ERROR("CombatWarriorField::Initialize() failed.");
    CombatWarriorFieldPool::GetInstance()->Deallocate(combat_warrior_field);
    return false;
  }

  this->combat_field_->AddWarrior(combat_warrior_field);

  require::RequireSceneBuild require_message;
  require_message.set_combat_id(this->combat_field_->GetId());
  require_message.set_building_id(action.building_id());
  require_message.set_warrior_id(combat_warrior_field->GetId());
  if (this->app_server_->GetRequireDispatcher()->Dispatch(
        require::REQUIRE_SCENE_BUILD, &require_message) < 0) {
    MYSYA_ERROR("require REQUIRE_SCENE_BUILD failed.");
    this->combat_field_->RemoveWarrior(combat_warrior_field->GetId());
    combat_warrior_field->Finalize();
    CombatWarriorFieldPool::GetInstance()->Deallocate(combat_warrior_field);
    return false;
  }

  combat_warrior_field->DispatchBuildActionEvent(action.building_id());

  return true;
}

bool CombatRoleField::DoMoveAction(const ::protocol::CombatMoveAction &action) {
  // Require.
  require::RequireSceneMove require_message;
  require_message.set_combat_id(this->combat_field_->GetId());
  *require_message.mutable_dest_pos() = action.pos();
  for (int i = 0; i < action.warrior_id_size(); ++i) {
    require_message.set_warrior_id(action.warrior_id(i));
    this->app_server_->GetRequireDispatcher()->Dispatch(
        require::REQUIRE_SCENE_MOVE, &require_message);
  }

  return true;
}

bool CombatRoleField::DoLockTargetAction(const ::protocol::CombatLockTargetAction &action) {
  // TODO:
  return false;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
