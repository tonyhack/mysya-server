#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_EVENT_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_EVENT_HANDLER_H

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

class CombatEventHandler {
  typedef ::google::protobuf::Message ProtoMessage;

 public:
  CombatEventHandler();
  ~CombatEventHandler();

  bool Initialize();
  void Finalize();

 private:
  void EventCombatBuildAction(const ProtoMessage *data);
  void EventCombatMoveAction(const ProtoMessage *data);

  uint64_t event_token_build_action_;
  uint64_t event_token_move_action_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatEventHandler);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_EVENT_HANDLER_H
