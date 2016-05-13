#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_ENTITY_BUILDER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_ENTITY_BUILDER_H

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

class Building;
class SceneApp;
class Warrior;

class EntityBuilder {
 public:
  EntityBuilder(SceneApp *host);
  ~EntityBuilder();

  Building *AllocateBuilding();
  void DeallocateBuilding(Building *building);

  Warrior *AllocateWarrior();
  void DeallocateWarrior(Warrior *warrior);

 private:
  SceneApp *host_;
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_ENTITY_BUILDER_H
