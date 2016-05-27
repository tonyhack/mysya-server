#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_REQUIRE_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_REQUIRE_HANDLER_H

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

class RequireHandler {
  typedef ::google::protobuf::Message ProtoMessage;

 public:
  RequireHandler();
  ~RequireHandler();

  bool Initialize();
  void Finalize();

 private:
  int OnRequireSceneBuild(ProtoMessage *data);
  int OnRequireSceneMove(ProtoMessage *data);
  int OnRequireSceneFetch(ProtoMessage *data);

  MYSYA_DISALLOW_COPY_AND_ASSIGN(RequireHandler);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_REQUIRE_HANDLER_H
