#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_H

#include <stdint.h>

#include <unordered_map>

#include "tutorial/orcas/combat/server/scene/grid.h"
#include "tutorial/orcas/protocol/cc/position.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class CombatField;

namespace scene {

class Building;
class Grid;
class Warrior;

class Scene {
 public:
  typedef Grid::BuildingSet BuildingSet;
  typedef Grid::WarriorSet WarriorSet;

  typedef std::unordered_map<int32_t, Building *> BuildingHashmap;
  typedef std::unordered_map<int32_t, Warrior *> WarriorHashmap;

  typedef std::vector< ::protocol::Position> PositionVector;

  Scene();
  ~Scene();

  bool Allocate(int map_id);
  void Deallocate();

  bool Initialize(CombatField *host);
  void Finalize();

  int32_t GetId() const;
  int32_t GetMapId() const;
  int32_t GetHeight() const;
  int32_t GetWidth() const;
  int32_t GetGridSize() const;

  CombatField *GetHost();

  Grid *GetGrid(const ::protocol::Position &pos);
  const Grid *GetGrid(const ::protocol::Position &pos) const;

  bool GetWalkable(const ::protocol::Position &pos) const;
  bool GetWalkable(int32_t x, int32_t y) const;

  bool AddBuilding(Building *building);
  void RemoveBuilding(Building *building);
  Building *RemoveBuilding(int32_t id);
  Building *GetBuilding(int32_t id);
  BuildingSet *GetBuildings(const ::protocol::Position &pos);
  const BuildingSet *GetBuildings(const ::protocol::Position &pos) const;

  bool AddWarrior(Warrior *warrior);
  void RemoveWarrior(Warrior *warrior);
  Warrior *RemoveWarrior(int32_t id);
  Warrior *GetWarrior(int32_t id);
  WarriorSet *GetWarriors(const ::protocol::Position &pos);
  const WarriorSet *GetWarriors(const ::protocol::Position &pos) const;

  void SearchPath(const ::protocol::Position &src_pos,
      const ::protocol::Position &dest_pos, PositionVector &paths);

 private:
  int32_t map_id_;
  CombatField *host_;

  int32_t height_;
  int32_t width_;
  int32_t grid_size_;

  Grid *grids_;

  BuildingHashmap buildings_;
  WarriorHashmap warriors_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Scene);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_H
