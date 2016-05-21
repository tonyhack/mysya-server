#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_COMBAT_REQUIRE_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_COMBAT_REQUIRE_HANDLER_H

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

class CombatRequireHandler {
  typedef ::google::protobuf::Message ProtoMessage;

 public:
  CombatRequireHandler();
  ~CombatRequireHandler();

  bool Initialize();
  void Finalize();

 private:
  int OnRequireCombatBuildAction(ProtoMessage *data);
  int OnRequireCombatMoveAction(ProtoMessage *data);

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatRequireHandler);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_COMBAT_REQUIRE_HANDLER_H
