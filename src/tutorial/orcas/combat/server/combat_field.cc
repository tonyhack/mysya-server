#include "tutorial/orcas/combat/server/combat_field.h"

#include <google/protobuf/message.h>

#include "tutorial/orcas/combat/server/app_session.h"
#include "tutorial/orcas/combat/server/combat_role_field_manager.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

CombatField::CombatField() : id_(0), app_session_(NULL) {}
CombatField::~CombatField() {}

bool CombatField::Initialize(AppSession *session) {
  this->app_session_ = session;
  this->app_session_->Add(this);

  return true;
}

void CombatField::Finalize() {
  if (this->app_session_ != NULL) {
    this->app_session_->Remove(this);
  }

  for (CombatRoleFieldSet::const_iterator iter = this->roles_.begin();
      iter != this->roles_.end(); ++iter) {
    CombatRoleField *role = CombatRoleFieldManager::GetInstance()->Remove(*iter);
    if (role != NULL) {
      CombatRoleFieldManager::GetInstance()->Deallocate(role);
    }
  }

  this->roles_.clear();
}

int32_t CombatField::GetId() const {
  return this->id_;
}

void CombatField::SetId(int32_t value) {
  this->id_ = value;
}

void CombatField::AddRole(uint64_t role_argent_id) {
  this->roles_.insert(role_argent_id);
}

void CombatField::RemoveRole(uint64_t role_argent_id) {
  this->roles_.erase(role_argent_id);
}

const CombatField::CombatRoleFieldSet &CombatField::GetRoles() const {
  return this->roles_;
}

void CombatField::ResetAppSession() {
  this->app_session_ = NULL;
}

int CombatField::SendMessage(const ::google::protobuf::Message &message) {
  if (this->app_session_ == NULL) {
    return -1;
  }

  return this->app_session_->SendMessage(message);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
