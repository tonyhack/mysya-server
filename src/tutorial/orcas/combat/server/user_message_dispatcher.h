#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_USER_MESSAGE_DISPATCHER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_USER_MESSAGE_DISPATCHER_H

#include <stdint.h>

#include <functional>
#include <unordered_map>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatRoleField;

class UserMessageDispatcher {
 public:
  typedef std::function<void (CombatRoleField *, const char *data,
      int size)> MessageCallback;
  typedef std::unordered_map<int32_t, MessageCallback>
      MessageCallbackHashmap;

  UserMessageDispatcher();
  ~UserMessageDispatcher();

  void SetMessageCallback(int32_t type, const MessageCallback &cb);
  void ResetMessageCallback(int32_t type);

  void Dispatch(int32_t type, CombatRoleField *role,
      const char *data, int size);

 private:
  MessageCallbackHashmap cbs_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(UserMessageDispatcher);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_USER_MESSAGE_DISPATCHER_H
