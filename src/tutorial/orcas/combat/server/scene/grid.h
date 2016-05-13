#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_GRID_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_GRID_H

#include <set>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

class Building;
class Warrior;

class Grid {
 public:
  typedef std::set<Building *> BuildingSet;
  typedef std::set<Warrior *> WarriorSet;

  Grid();
  ~Grid();

  bool GetWalkable() const;
  void SetWalkable(bool value);

  bool AddBuilding(Building *building);
  void RemoveBuilding(Building *building);
  BuildingSet *GetBuildings();
  const BuildingSet *GetBuildings() const;

  bool AddWarrior(Warrior *warrior);
  void RemoveWarrior(Warrior *warrior);
  WarriorSet *GetWarriors();
  const WarriorSet *GetWarriors() const;

 private:
  bool walkable_;

  BuildingSet buildings_;
  WarriorSet warriors_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Grid);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_GRID_H
