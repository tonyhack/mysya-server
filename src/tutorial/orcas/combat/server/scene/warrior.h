#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_WARRIOR_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_WARRIOR_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/scene/move_action.h"
#include "tutorial/orcas/protocol/cc/position.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatWarriorField;

namespace scene {

class Scene;

class Warrior {
 public:
  typedef MoveAction::PositionVector PositionVector;

  Warrior();
  ~Warrior();

  bool Initialize(CombatWarriorField *host, Scene *scene);
  void Finalize();

  int32_t GetId() const;
  Scene *GetScene();

  CombatWarriorField *GetHost();
  ::protocol::Position GetPos();
  int32_t GetMoveSpeed() const;
  MoveAction *GetMoveAction();

  void StartMove(const ::protocol::Position &dest_pos);
  void DispatchMoveActionEvent(const ::protocol::Position &dest_pos,
      const PositionVector &path);

 private:
  Scene *scene_;
  CombatWarriorField *host_;
  MoveAction move_action_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Warrior);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_WARRIOR_H
