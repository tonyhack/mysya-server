#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_USER_MESSAGE_HANDLER_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_USER_MESSAGE_HANDLER_H

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

class Actor;
class AppServer;

class UserMessageHandler {
 public:
  UserMessageHandler(AppServer *host);
  ~UserMessageHandler();

  void SetMessageHandlers();
  void ResetMessageHandlers();

 private:
  void OnMessageLoginRequest(Actor *actor, const char *data, int size);
  void OnMessageCombatRequest(Actor *actor, const char *data, int size);
  void OnMessageCombatActionRequest(Actor *actor, const char *data, int size);

  AppServer *host_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(UserMessageHandler);
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_USER_MESSAGE_HANDLER_H
