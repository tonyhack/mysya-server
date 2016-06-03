#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_MESSAGE_HANDLER_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_MESSAGE_HANDLER_H

#include <mysya/util/class_util.h>

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {

namespace combat {

class TransportChannel;

}  // namespace combat

namespace gateway {
namespace server {

class AppServer;

class CombatMessageHandler {
 public:
  typedef ::tutorial::orcas::combat::TransportChannel TransportChannel;
  typedef ::google::protobuf::Message ProtoMessage;

  CombatMessageHandler(AppServer *host);
  ~CombatMessageHandler();

  void SetMessageHandlers();
  void ResetMessageHandlers();

 private:
  void OnMessageCombatDeployResponse(TransportChannel *channel,
      const ProtoMessage *data);
  void OnMessageCombatConnectArgentResponse(TransportChannel *channel,
      const ProtoMessage *data);
  void OnMessageCombatBeginResponse(TransportChannel *channel,
      const ProtoMessage *data);
  void OnMessageCombatArgentSync(TransportChannel *channel,
      const ProtoMessage *data);
  void OnMessageCombatSettlementSync(TransportChannel *channel,
      const ProtoMessage *data);

  AppServer *host_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatMessageHandler);
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_MESSAGE_HANDLER_H
