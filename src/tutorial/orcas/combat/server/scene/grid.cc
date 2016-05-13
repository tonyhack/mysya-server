#include "tutorial/orcas/combat/server/scene/grid.h"

#include "tutorial/orcas/combat/server/scene/building.h"
#include "tutorial/orcas/combat/server/scene/warrior.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

Grid::Grid() : walkable_(false) {}
Grid::~Grid() {
  this->buildings_.clear();
  this->warriors_.clear();
}

bool Grid::GetWalkable() const {
  return this->walkable_ && this->buildings_.empty() == false;
}

void Grid::SetWalkable(bool value) {
  this->walkable_ = value;
}

bool Grid::AddBuilding(Building *building) {
  return this->buildings_.insert(building).second;
}

void Grid::RemoveBuilding(Building *building) {
  this->buildings_.erase(building);
}

Grid::BuildingSet *Grid::GetBuildings() {
  return &this->buildings_;
}

const Grid::BuildingSet *Grid::GetBuildings() const {
  return &this->buildings_;
}

bool Grid::AddWarrior(Warrior *warrior) {
  return this->warriors_.insert(warrior).second;
}

void Grid::RemoveWarrior(Warrior *warrior) {
  this->warriors_.erase(warrior);
}

Grid::WarriorSet *Grid::GetWarriors() {
  return &this->warriors_;
}

const Grid::WarriorSet *Grid::GetWarriors() const {
  return &this->warriors_;
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
