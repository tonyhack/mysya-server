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

class SceneApp;

class CombatEventHandler {
  typedef ::google::protobuf::Message ProtoMessage;

 public:
  CombatEventHandler();
  ~CombatEventHandler();

  bool Initialize(SceneApp *host);
  void Finalize();

 private:
  void OnEventCombatBegin(const ProtoMessage *data);

  SceneApp *host_;

  uint64_t event_token_begin_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatEventHandler);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_COMBAT_EVENT_HANDLER_H
