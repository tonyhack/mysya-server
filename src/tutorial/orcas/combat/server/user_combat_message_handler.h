#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_USER_COMBAT_MESSAGE_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_USER_COMBAT_MESSAGE_HANDLER_H

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace combat {

class TransportChannel;

namespace server {

class AppServer;
class CombatRoleField;

class UserCombatMessageHandler {
 public:
  explicit UserCombatMessageHandler(AppServer *host);
  ~UserCombatMessageHandler();

  void SetMessageHandlers();
  void ResetMessageHandlers();

 private:
  void OnMessageCombatActionRequest(CombatRoleField *role,
      const char *data, int size);

  AppServer *host_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(UserCombatMessageHandler);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_USER_COMBAT_MESSAGE_HANDLER_H
