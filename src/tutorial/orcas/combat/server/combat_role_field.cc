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
  : building_num_(0), combat_field_(NULL),
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

  this->fields_.Clear();
}

AppServer *CombatRoleField::GetAppServer() {
  return this->app_server_;
}

uint64_t CombatRoleField::GetArgentId() const {
  return this->fields_.id();
}

void CombatRoleField::SetArgentId(uint64_t value) {
  this->fields_.set_id(value);
}

int32_t CombatRoleField::GetCampId() const {
  return this->fields_.camp_id();
}

void CombatRoleField::SetCampId(int32_t value) {
  this->fields_.set_camp_id(value);
}

::protocol::CombatRoleFields &CombatRoleField::GetFields() {
  return this->fields_;
}

const ::protocol::CombatRoleFields &CombatRoleField::GetFields() const {
  return this->fields_;
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

int32_t CombatRoleField::GetFood() const {
  return this->fields_.food();
}

void CombatRoleField::SetFood(int32_t value) {
  if (value > this->GetFoodMax()) {
    this->fields_.set_food(this->GetFoodMax());
  } else {
    this->fields_.set_food(value);
  }
}

void CombatRoleField::IncFood(int32_t increment) {
  this->SetFood(this->GetFood() + increment);
}

int32_t CombatRoleField::GetFoodMax() const {
  return this->fields_.food_max();
}

void CombatRoleField::SetFoodMax(int32_t value) {
  this->fields_.set_food_max(value);
}

void CombatRoleField::IncFoodMax(int32_t increment) {
  this->SetFoodMax(this->GetFoodMax() + increment);
}

int32_t CombatRoleField::GetSupplyMax() const {
  return this->fields_.supply_max();
}

void CombatRoleField::SetSupplyMax(int32_t value) {
  this->fields_.set_supply_max(value);
}

void CombatRoleField::IncSupplyMax(int32_t increment) {
  this->SetSupplyMax(this->GetSupplyMax() + increment);
}

int32_t CombatRoleField::GetSupply() const {
  return this->fields_.supply();
}

void CombatRoleField::SetSupply(int32_t value) {
  this->fields_.set_supply(value);
}

void CombatRoleField::IncSupply(int32_t increment) {
  this->SetSupply(this->GetSupply() + increment);
}

int32_t CombatRoleField::GetElixir() const {
  return this->fields_.elixir();
}

void CombatRoleField::SetElixir(int32_t value) {
  if (value > this->GetElixirMax()) {
    this->fields_.set_elixir(this->GetElixirMax());
  } else {
    this->fields_.set_elixir(value);
  }
}

void CombatRoleField::IncElixir(int32_t increment) {
  this->SetElixir(this->GetElixir() + increment);
}

int32_t CombatRoleField::GetElixirMax() const {
  return this->fields_.elixir_max();
}

void CombatRoleField::SetElixirMax(int32_t value) {
  this->fields_.set_elixir_max(value);
}

void CombatRoleField::IncElixirMax(int32_t increment) {
  this->SetElixirMax(this->GetElixirMax() + increment);
}

CombatField *CombatRoleField::GetCombatField() {
  return this->combat_field_;
}

void CombatRoleField::SetCombatField(CombatField *value) {
  this->combat_field_ = value;
}

void CombatRoleField::AddWarriorDescription(
    const ::protocol::WarriorDescription &description) {
  this->warrior_descriptions_.insert(std::make_pair(description.id(), description));
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

  if (this->GetFood() < warrior_description->food_need()) {
    MYSYA_ERROR("CombatRoleField::GetFood(%d) < WarriorDescription::food_need(%d).",
        this->GetFood(), warrior_description->food_need());
    return false;
  }

  int32_t available_supply = this->GetSupplyMax() - this->GetSupply();
  if (available_supply < warrior_description->supply_need()) {
    MYSYA_ERROR("CombatRoleField's available_supply(%d) < WarriorDescription::supply_need(%d).",
        available_supply, warrior_description->supply_need());
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

  this->IncSupply(warrior_description->supply_need());
  this->IncFood(0 - warrior_description->food_need());

  combat_warrior_field->DispatchBuildActionEvent(action.building_id());

  this->combat_field_->PrintRoleResources();

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
