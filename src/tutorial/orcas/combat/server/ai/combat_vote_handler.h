#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_COMBAT_VOTE_HANDLER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_COMBAT_VOTE_HANDLER_H

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

class CombatVoteHandler {
 public:
  typedef ::google::protobuf::Message ProtoMessage;

  CombatVoteHandler();
  ~CombatVoteHandler();

  bool Initialize();
  void Finalize();

 private:
  int OnVoteCombatMoveAction(const ProtoMessage *data);

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatVoteHandler);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_COMBAT_VOTE_HANDLER_H
