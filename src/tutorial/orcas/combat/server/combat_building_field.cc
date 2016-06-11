#include "tutorial/orcas/combat/server/combat_building_field.h"

#include "tutorial/orcas/combat/server/combat_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

CombatBuildingField::CombatBuildingField()
  : role_argent_id_(0), host_combat_(NULL) {}
CombatBuildingField::~CombatBuildingField() {}

bool CombatBuildingField::Initialize(int32_t id, CombatField *host_combat,
    int64_t role_argent_id, int32_t camp_id,
    const ::protocol::BuildingDescription &description) {
  this->host_combat_ = host_combat;
  this->role_argent_id_ = role_argent_id;

  this->fields_.set_id(id);
  this->fields_.set_conf_id(description.id());
  this->fields_.set_type(description.type());
  this->fields_.set_host_id(this->role_argent_id_);
  this->fields_.set_camp_id(camp_id);
  this->fields_.set_hp(description.hp());
  this->fields_.set_max_hp(description.hp());
  this->fields_.set_pos_x(description.pos_x());
  this->fields_.set_pos_y(description.pos_y());
  this->fields_.set_food_add(description.food_add());
  this->fields_.set_supply(description.supply());
  this->fields_.set_elixir_add(description.elixir_add());
  this->fields_.set_hp_recovery(description.hp_recovery());
  this->fields_.set_status(::protocol::BUILDING_STATUE_TYPE_HOST);

  return true;
}

void CombatBuildingField::Finalize() {
  this->host_combat_ = NULL;
  this->role_argent_id_ = 0;
  this->fields_.Clear();
}

int32_t CombatBuildingField::GetId() const {
  return this->fields_.id();
}

int32_t CombatBuildingField::GetFoodAdd() const {
  return this->fields_.food_add();
}

void CombatBuildingField::SetFoodAdd(int32_t value) {
  this->fields_.set_food_add(value);
}

void CombatBuildingField::IncFoodAdd(int32_t increment) {
  this->SetFoodAdd(this->GetFoodAdd() + increment);
}

int32_t CombatBuildingField::GetSupply() const {
  return this->fields_.supply();
}

void CombatBuildingField::SetSupply(int32_t value) {
  this->fields_.set_supply(value);
}

void CombatBuildingField::IncSupply(int32_t increment) {
  this->SetSupply(this->GetSupply() + increment);
}

int32_t CombatBuildingField::GetElixirAdd() const {
  return this->fields_.elixir_add();
}

void CombatBuildingField::SetElixirAdd(int32_t value) {
  this->fields_.set_elixir_add(value);
}

void CombatBuildingField::IncElixirAdd(int32_t increment) {
  this->SetElixirAdd(this->GetElixirAdd() + increment);
}

CombatField *CombatBuildingField::GetCombatField() {
  return this->host_combat_;
}

int32_t CombatBuildingField::GetCombatRoleArgentId() const {
  return this->role_argent_id_;
}

::protocol::CombatBuildingFields &CombatBuildingField::GetFields() {
  return this->fields_;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
