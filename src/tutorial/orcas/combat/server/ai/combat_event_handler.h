#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_COMBAT_EVENT_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_COMBAT_EVENT_HANDLER_H

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
namespace ai {

class CombatEventHandler {
  typedef ::google::protobuf::Message ProtoMessage;

 public:
  CombatEventHandler();
  ~CombatEventHandler();

  bool Initialize();
  void Finalize();

 private:
  void OnEventCombatBegin(const ProtoMessage *data);
  void OnEventCombatBuildAction(const ProtoMessage *data);
  void OnEventCombatDeath(const ProtoMessage *data);
  void OnEventCombatConvertCamp(const ProtoMessage *data);
  void OnEventCombatAttacked(const ProtoMessage *data);
  void OnEventCombatSettle(const ProtoMessage *data);
  void OnEventCombatResourceRecover(const ProtoMessage *data);

  uint64_t event_token_begin_;
  uint64_t event_token_build_action_;
  uint64_t event_token_death_;
  uint64_t event_token_convert_camp_;
  uint64_t event_token_attacked_;
  uint64_t event_token_settle_;
  uint64_t event_token_resource_recover_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatEventHandler);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_COMBAT_EVENT_HANDLER_H
