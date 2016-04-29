#include "tutorial/orcas/combat/server/warrior_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

WarriorField::WarriorField() {}

WarriorField::~WarriorField() {}

bool WarriorField::Initialize(uint64_t host_id, int32_t camp_id,
    const ::protocol::WarriorDescription &description) {
  this->fields_.set_id(description.id());
  this->fields_.set_type(description.type());
  this->fields_.set_host_id(host_id);
  this->fields_.set_camp_id(camp_id);
  this->fields_.set_hp(0);
  this->fields_.set_max_hp(0);
  this->fields_.set_attack(0);
  this->fields_.set_defence(0);
  this->fields_.set_attack_speed(0);
  this->fields_.set_move_speed(0);

  this->junior_fields_.set_max_hp(description.max_hp());
  this->junior_fields_.set_attack(description.attack());
  this->junior_fields_.set_defence(description.defence());
  this->junior_fields_.set_move_speed(description.attack_speed());
  this->junior_fields_.set_attack_speed(description.move_speed());
  this->junior_fields_.set_attack_range(description.attack_range());

  this->senior_fields_.set_max_hp_add_value(0);
  this->senior_fields_.set_max_hp_add_per(0);
  this->senior_fields_.set_attack_add_value(0);
  this->senior_fields_.set_attack_add_per(0);
  this->senior_fields_.set_defence_add_value(0);
  this->senior_fields_.set_defence_add_per(0);
  this->senior_fields_.set_move_speed_add_value(0);
  this->senior_fields_.set_move_speed_add_per(0);
  this->senior_fields_.set_attack_speed_add_value(0);
  this->senior_fields_.set_attack_speed_add_per(0);

  return true;
}

void WarriorField::Finalize() {
}

int32_t WarriorField::GetId() const {
  return this->fields_.id();
}

uint64_t WarriorField::GetHostId() const {
  return this->fields_.host_id();
}

void WarriorField::SetHostId(uint64_t value) {
  this->fields_.set_host_id(value);
}

int32_t WarriorField::GetCampId() const {
  return this->fields_.camp_id();
}

void WarriorField::SetCampId(int32_t value) {
  this->fields_.set_camp_id(value);
}

::protocol::WarriorFields &WarriorField::GetFields() {
  return this->fields_;
}

::protocol::WarriorJuniorFields &WarriorField::GetJuniorFields() {
  return this->junior_fields_;
}

::protocol::WarriorSeniorFields &WarriorField::GetSeniorFields() {
  return this->senior_fields_;
}

void WarriorField::GenerateFields() {}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
