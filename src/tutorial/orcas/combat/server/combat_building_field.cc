#include "tutorial/orcas/combat/server/combat_building_field.h"

#include "tutorial/orcas/combat/server/combat_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

CombatBuildingField::CombatBuildingField()
  : host_(NULL) {}
CombatBuildingField::~CombatBuildingField() {}

bool CombatBuildingField::Initialize(int32_t id, CombatField *host,
    int32_t camp_id, const ::protocol::BuildingDescription &description) {
  this->host_ = host;

  this->fields_.set_id(id);
  this->fields_.set_conf_id(description.id());
  this->fields_.set_camp_id(camp_id);
  this->fields_.set_hp(description.hp());
  this->fields_.set_max_hp(description.hp());
  this->fields_.set_pos_x(description.pos_x());
  this->fields_.set_pos_y(description.pos_y());

  return true;
}

void CombatBuildingField::Finalize() {
  this->host_ = NULL;
  this->fields_.Clear();
}

int32_t CombatBuildingField::GetId() const {
  return this->fields_.id();
}

CombatField *CombatBuildingField::GetHost() {
  return this->host_;
}

::protocol::CombatBuildingFields &CombatBuildingField::GetFields() {
  return this->fields_;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
