#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_COMBAT_EVENT_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_COMBAT_EVENT_HANDLER_H

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
namespace scene {

class CombatEventHandler {
  typedef ::google::protobuf::Message ProtoMessage;

 public:
  CombatEventHandler();
  ~CombatEventHandler();

  bool Initialize();
  void Finalize();

 private:
  void OnEventCombatBegin(const ProtoMessage *data);
  void OnEventCombatDeath(const ProtoMessage *data);
  void OnEventCombatLockTarget(const ProtoMessage *data);
  void OnEventCombatSettle(const ProtoMessage *data);

  uint64_t event_token_begin_;
  uint64_t event_token_death_;
  uint64_t event_token_lock_target_;
  uint64_t event_token_settle_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatEventHandler);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_COMBAT_EVENT_HANDLER_H
