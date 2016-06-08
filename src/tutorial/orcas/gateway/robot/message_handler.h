#ifndef TUTORIAL_ORCAS_GATEWAY_ROBOT_MESSAGE_HANDLER_H
#define TUTORIAL_ORCAS_GATEWAY_ROBOT_MESSAGE_HANDLER_H

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace gateway {
namespace robot {

class Actor;

class MessageHandler {
 public:
  MessageHandler();
  ~MessageHandler();

  void SetHandlers();
  void ResetHandlers();

 private:
  void OnMessageLoginResponse(Actor *actor, const char *data, int size);
  void OnMessageCombatResponse(Actor *actor, const char *data, int size);
  void OnMessageCombatBeginSync(Actor *actor, const char *data, int size);
  void OnMessageCombatActionResponse(Actor *actor, const char *data, int size);
  void OnMessageCombatActionSync(Actor *actor, const char *data, int size);
  void OnMessageCombatSettlementSync(Actor *actor, const char *data, int size);

  MYSYA_DISALLOW_COPY_AND_ASSIGN(MessageHandler);
};

}  // namespace robot
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_ROBOT_MESSAGE_HANDLER_H
