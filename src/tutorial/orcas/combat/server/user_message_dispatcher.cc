#include "tutorial/orcas/combat/server/user_message_dispatcher.h"

#include "tutorial/orcas/combat/server/combat_role_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

UserMessageDispatcher::UserMessageDispatcher() {}
UserMessageDispatcher::~UserMessageDispatcher() {}

void UserMessageDispatcher::SetMessageCallback(int32_t type,
    const MessageCallback &cb) {
  this->cbs_[type] = cb;
}

void UserMessageDispatcher::ResetMessageCallback(int32_t type) {
  this->cbs_.erase(type);
}

void UserMessageDispatcher::Dispatch(int32_t type, CombatRoleField *role,
    const char *data, int size) {
  MessageCallbackHashmap::iterator iter = this->cbs_.find(type);
  if (iter == this->cbs_.end()) {
    return;
  }

  iter->second(role, data, size);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
