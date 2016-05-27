#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_SCENE_EVENT_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_SCENE_EVENT_HANDLER_H

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

class SceneEventHandler {
 public:
  typedef ::google::protobuf::Message ProtoMessage;

  SceneEventHandler();
  ~SceneEventHandler();

  bool Initialize();
  void Finalize();

 private:
  void OnEventSceneMoveStep(const ProtoMessage *data);

  uint64_t event_token_scene_move_step_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(SceneEventHandler);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_SCENE_EVENT_HANDLER_H
