#include "tutorial/orcas/combat/server/combat_role_field.h"

#include <google/protobuf/message.h>

#include "tutorial/orcas/combat/server/app_session.h"
#include "tutorial/orcas/combat/server/warrior_field.h"
#include "tutorial/orcas/combat/server/warrior_field_pool.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

CombatRoleField::CombatRoleField()
  : argent_id_(0), camp_id_(0),
    combat_field_(NULL), app_session_(NULL) {}

CombatRoleField::~CombatRoleField() {}

bool CombatRoleField::Initialize(uint64_t argent_id) {
  this->SetArgentId(argent_id);

  return true;
}

void CombatRoleField::Finalize() {
  if (this->app_session_ != NULL) {
    this->app_session_->Remove(this);
  }

  for (WarriorFieldMap::iterator iter = this->warrior_fields_.begin();
      iter != this->warrior_fields_.end(); ++iter) {
    iter->second->Finalize();
    WarriorFieldPool::GetInstance()->Deallocate(iter->second);
  }

  this->warrior_fields_.clear();

  this->SetArgentId(0);
  this->SetCampId(0);
  this->SetCombatField(NULL);
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

CombatField *CombatRoleField::GetCombatField() {
  return this->combat_field_;
}

void CombatRoleField::SetCombatField(CombatField *value) {
  this->combat_field_ = value;
}

bool CombatRoleField::AddWarriorField(WarriorField *warrior) {
  WarriorFieldMap::iterator iter =
    this->warrior_fields_.find(warrior->GetId());
  if (iter != this->warrior_fields_.end()) {
    return false;
  }

  this->warrior_fields_.insert(std::make_pair(warrior->GetId(), warrior));
  return true;
}

WarriorField *CombatRoleField::GetWarriorField(int32_t id) {
  WarriorField *warrior = NULL;

  WarriorFieldMap::iterator iter = this->warrior_fields_.find(id);
  if (iter != this->warrior_fields_.end()) {
    warrior = iter->second;
  }

  return warrior;
}

WarriorField *CombatRoleField::RemoveWarriorField(int32_t id) {
  WarriorField *warrior = NULL;

  WarriorFieldMap::iterator iter = this->warrior_fields_.find(id);
  if (iter != this->warrior_fields_.end()) {
    warrior = iter->second;
    this->warrior_fields_.erase(iter);
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

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
