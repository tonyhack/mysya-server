#include "tutorial/orcas/combat/server/combat_role_field.h"

#include <google/protobuf/message.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/app_session.h"
#include "tutorial/orcas/combat/server/require/cc/require.pb.h"
#include "tutorial/orcas/combat/server/require/cc/require_combat.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

CombatRoleField::CombatRoleField()
  : argent_id_(0), camp_id_(0),
    combat_field_(NULL), app_session_(NULL),
    app_server_(NULL) {}

CombatRoleField::~CombatRoleField() {}

bool CombatRoleField::Initialize(uint64_t argent_id,
    AppServer *app_server) {
  this->SetArgentId(argent_id);
  this->app_server_ = app_server;
  this->app_session_ = NULL;

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
    case ::protocol::COMBAT_ACTION_TYPE_ATTACK:
      result = this->DoAttackAction(action.attack_action());
      break;
    default:
      break;
  }

  return result;
}

bool CombatRoleField::DoBuildAction(const ::protocol::CombatBuildAction &action) {
  // TODO:
  return false;
}

bool CombatRoleField::DoMoveAction(const ::protocol::CombatMoveAction &action) {
  require::RequireCombatMoveAction message;
  *message.mutable_action() = action;
  return this->app_server_->GetRequireDispatcher()->Dispatch(
      require::REQUIRE_COMBAT_MOVE_ACTION, &message) >= 0;
}

bool CombatRoleField::DoAttackAction(const ::protocol::CombatAttackAction &action) {
  // TODO:
  return false;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
