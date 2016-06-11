#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_VOTE_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_VOTE_HANDLER_H

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

class VoteHandler {
 public:
  typedef ::google::protobuf::Message ProtoMessage;

  VoteHandler();
  ~VoteHandler();

  bool Initialize();
  void Finalize();

 private:
  int OnVoteSceneMove(const ProtoMessage *data);
  int OnVoteCombatBuild(const ProtoMessage *data);

  uint64_t token_scene_move_;
  uint64_t token_combat_build_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(VoteHandler);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_VOTE_HANDLER_H
