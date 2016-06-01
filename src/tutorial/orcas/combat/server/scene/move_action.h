#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_MOVE_ACTION_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_MOVE_ACTION_H

#include <vector>

#include "tutorial/orcas/protocol/cc/position.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

class Warrior;

class MoveAction {
 public:
  typedef std::vector< ::protocol::Position> PositionVector;

  MoveAction();
  ~MoveAction();

  bool Initialize(Warrior *host);
  void Finalize();

  void Start(const ::protocol::Position &dest_pos);
  void Reset();
  void Finish(bool success = true);

 private:
  void OnMoveTimer(int64_t timer_id);
  void MoveStep();

  ::protocol::Position dest_pos_;
  PositionVector move_paths_;
  int path_index_;
  Warrior *host_;
  int32_t move_ms_;
  int64_t timer_move_token_;
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_MOVE_ACTION_H
