#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_REQUIRE_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_REQUIRE_HANDLER_H

#include <stdint.h>

#include <mysya/util/class_util.h>

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppServer;

class RequireHandler {
  typedef ::google::protobuf::Message ProtoMessage;

 public:
  RequireHandler(AppServer *host);
  ~RequireHandler();

  bool SetHandlers();
  void ResetHandlers();

 private:
  int OnRequireCombatSettle(ProtoMessage *data);

  AppServer *host_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(RequireHandler);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_REQUIRE_HANDLER_H
