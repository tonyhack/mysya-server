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
  typedef ::protocol::Position Position;

  class Node {
   public:
    bool operator <(const Node &other) const;
    int NeighbroGCost(const Node &other) const;
    int HeursiticConstEstimate(const Node &other) const;
  
    Position pos_;
    bool walkable_;
    int f_;
    int g_;
    int open_list_pos_;
    int close_list_pos_;
    Node *parent_;
  };

 public:
  typedef std::vector<Node> NodeVector;
  typedef std::vector<Node *> NodePtrVector;

  typedef std::vector<Building *> BuildingVector;
  typedef std::vector<Warrior *> WarriorVector;

  typedef Grid::BuildingSet BuildingSet;
  typedef Grid::WarriorSet WarriorSet;

  typedef std::unordered_map<int32_t, Building *> BuildingHashmap;
  typedef std::unordered_map<int32_t, Warrior *> WarriorHashmap;

  typedef std::vector<Position> PositionVector;

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

  Grid *GetGrid(const Position &pos);
  Grid *GetGrid(int32_t x, int32_t y);
  const Grid *GetGrid(const Position &pos) const;
  const Grid *GetGrid(int32_t x, int32_t y) const;

  bool GetWalkable(const Position &pos) const;
  bool GetWalkable(int32_t x, int32_t y) const;

  bool GetNearlyWalkablePos(const Position &pos, Position &nearly_pos) const;
  bool GetNearlyWalkablePos(int32_t x, int32_t y, Position &nearly_pos) const;

  bool AddBuilding(Building *building);
  void RemoveBuilding(Building *building);
  Building *RemoveBuilding(int32_t id);
  Building *GetBuilding(int32_t id);
  BuildingSet *GetBuildings(const Position &pos);
  const BuildingSet *GetBuildings(const Position &pos) const;

  bool AddWarrior(Warrior *warrior);
  void RemoveWarrior(Warrior *warrior);
  Warrior *RemoveWarrior(int32_t id);
  Warrior *GetWarrior(int32_t id);
  WarriorSet *GetWarriors(const Position &pos);
  const WarriorSet *GetWarriors(const Position &pos) const;

  bool GetNeighbors(const Position &pos, int32_t range,
      BuildingVector &buildings, WarriorVector &warriors);

  void SearchPath(const Position &src_pos, const Position &dest_pos,
      PositionVector &paths);
  void PrintSearchPath(const Position &src_pos, const Position &dest_pos);

  void PrintStatusImage();

 private:
  void OnTimerPrintStatusImage(int64_t timer_id);

  Node *GetNode(const Position &pos);
  Node *GetNode(int32_t x, int32_t y);
  void GetNeighborNodes(const Node *node, NodePtrVector &neighbor_nodes);
  bool IsInOpenList(const Node *node) const;
  bool IsOpenListEmpty() const;
  void InsertOpenList(Node *node);
  Node *GetMinFScoreNodeInOpenList();
  void DeleteMinFScoreNodeInOpenList();
  void ResortOpenList(Node *node);
  bool IsInCloseList(const Node *node) const;
  void InsertCloseList(Node *node);
  void ConstructResultPath(PositionVector &result);

  int32_t map_id_;
  CombatField *host_;

  int32_t height_;
  int32_t width_;
  int32_t grid_size_;

  Grid *grids_;

  BuildingHashmap buildings_;
  WarriorHashmap warriors_;

  NodeVector nodes_;
  Node *start_node_;
  Node *end_node_;
  NodePtrVector open_list_;

  int64_t timer_id_print_status_image_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Scene);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_H
