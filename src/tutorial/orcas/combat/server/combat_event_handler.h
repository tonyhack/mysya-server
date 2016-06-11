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

class AppServer;

class CombatEventHandler {
  typedef ::google::protobuf::Message ProtoMessage;

 public:
  CombatEventHandler(AppServer *host);
  ~CombatEventHandler();

  void SetHandlers();
  void ResetHandlers();

 private:
  void OnEventCombatBuildAction(const ProtoMessage *data);
  void OnEventCombatMoveAction(const ProtoMessage *data);
  void OnEventCombatDeath(const ProtoMessage *data);
  void OnEventCombatLockTarget(const ProtoMessage *data);
  void OnEventCombatConvertCamp(const ProtoMessage *data);
  void OnEventCombatBuildingSwitchStatus(const ProtoMessage *data);

  AppServer *host_;

  uint64_t event_token_build_action_;
  uint64_t event_token_move_action_;
  uint64_t event_token_death_;
  uint64_t event_token_lock_target_;
  uint64_t event_token_convert_camp_;
  uint64_t event_token_building_switch_status_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatEventHandler);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_EVENT_HANDLER_H
