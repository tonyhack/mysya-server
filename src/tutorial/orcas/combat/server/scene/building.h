#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_BUILDING_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_BUILDING_H

#include <mysya/util/class_util.h>
#include <tutorial/orcas/protocol/cc/position.pb.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatBuildingField;

namespace scene {

class Scene;

class Building {
 public:
  Building();
  ~Building();

  bool Initialize(CombatBuildingField *host, Scene *scene);
  void Finalize();

  int32_t GetId() const;
  ::protocol::Position GetPos();
  CombatBuildingField *GetHost();

 private:
  Scene *scene_;
  CombatBuildingField *host_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Building);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_BUILDING_H
