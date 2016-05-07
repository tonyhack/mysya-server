#include "tutorial/orcas/combat/server/warrior_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

WarriorField::WarriorField() {}

WarriorField::~WarriorField() {}

bool WarriorField::Initialize(uint64_t host_id, int32_t camp_id,
    const ::protocol::WarriorDescription &description) {
  this->SetDescription(description);
  this->SetHostId(host_id);
  this->SetCampId(camp_id);

  return true;
}

void WarriorField::Finalize() {}

int32_t WarriorField::GetId() const {
  return this->description_.id();
}

uint64_t WarriorField::GetHostId() const {
  return this->host_id_;
}

void WarriorField::SetHostId(uint64_t value) {
  this->host_id_ = value;
}

void WarriorFields::SetDescription(const ::protocol::WarriorDescription &value) {
  this->description_ = value;
}

const ::protocol::WarriorDescription &WarriorFields::GetDescription() const {
  return this->description_;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
