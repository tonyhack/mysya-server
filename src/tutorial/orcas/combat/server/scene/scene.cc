#include "tutorial/orcas/combat/server/scene/scene.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/scene/building.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"
#include "tutorial/orcas/combat/server/scene/scene_manager.h"
#include "tutorial/orcas/combat/server/scene/warrior.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

Scene::Scene()
  : map_id_(0), host_(NULL),
    height_(0), width_(0),
    grid_size_(0), grids_(NULL) {}

Scene::~Scene() {}

bool Scene::Allocate(int map_id) {
  const SceneConf *conf =
    SceneManager::GetInstance()->GetSceneConf(map_id);
  if (conf == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::GetMapBlocks(%d) failed.", map_id);
    return false;
  }

  this->grids_ = new Grid[conf->blocks_.size()];
  if (this->grids_ == NULL) {
    MYSYA_ERROR("[SCENE] allocate Grid[%lu] failed.", conf->blocks_.size());
    return false;
  }

  for (size_t i = 0; i < conf->blocks_.size(); ++i) {
    this->grids_[i].SetWalkable(conf->blocks_.data()[i] == '0');
  }

  this->map_id_ = map_id;
  this->height_ = conf->height_;
  this->width_ = conf->width_;
  this->grid_size_ = this->height_ * this->width_;

  return true;
}

void Scene::Deallocate() {
  delete [] this->grids_;
}

bool Scene::Initialize(CombatField *host) {
  this->host_ = host;

  return true;
}

void Scene::Finalize() {
  for (BuildingHashmap::iterator iter = this->buildings_.begin();
      iter != this->buildings_.end(); ++iter) {
    SceneApp::GetInstance()->GetEntityBuilder()->DeallocateBuilding(
        iter->second);
  }
  this->buildings_.clear();

  for (WarriorHashmap::iterator iter = this->warriors_.begin();
      iter != this->warriors_.end(); ++iter) {
    SceneApp::GetInstance()->GetEntityBuilder()->DeallocateWarrior(
        iter->second);
  }
  this->warriors_.clear();

  this->host_ = NULL;
}

int32_t Scene::GetId() const {
  return this->host_->GetId();
}

int32_t Scene::GetMapId() const {
  return this->map_id_;
}

int32_t Scene::GetHeight() const {
  return this->height_;
}

int32_t Scene::GetWidth() const {
  return this->width_;
}

int32_t Scene::GetGridSize() const {
  return this->grid_size_;
}

Grid *Scene::GetGrid(const ::protocol::Position &pos) {
  if (pos.x() < 0 || pos.x() >= this->width_ ||
      pos.y() < 0 || pos.y() >= this->height_) {
    return NULL;
  }

  return &this->grids_[pos.y()*this->height_+pos.x()];
}

const Grid *Scene::GetGrid(const ::protocol::Position &pos) const {
  if (pos.x() < 0 || pos.x() >= this->width_ ||
      pos.y() < 0 || pos.y() >= this->height_) {
    return NULL;
  }

  return &this->grids_[pos.y()*this->height_+pos.x()];
}

bool Scene::GetWalkable(const ::protocol::Position &pos) const {
  const Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return false;
  }

  return grid->GetWalkable();
}

bool Scene::GetWalkable(int32_t x, int32_t y) const {
  ::protocol::Position pos;
  pos.set_x(x);
  pos.set_y(y);

  return this->GetWalkable(pos);
}

bool Scene::AddBuilding(Building *building) {
  BuildingHashmap::iterator iter = this->buildings_.find(building->GetId());
  if (iter != this->buildings_.end()) {
    return false;
  }

  ::protocol::Position pos = building->GetPos();
  Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return false;
  }

  if (grid->AddBuilding(building) == false) {
    return false;
  }

  this->buildings_.insert(std::make_pair(building->GetId(), building));

  return true;
}

void Scene::RemoveBuilding(Building *building) {
  ::protocol::Position pos = building->GetPos();
  Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return;
  }

  grid->RemoveBuilding(building);
  this->buildings_.erase(building->GetId());
}

Building *Scene::RemoveBuilding(int32_t id) {
  Building *building = NULL;

  BuildingHashmap::iterator iter = this->buildings_.find(id);
  if (iter != this->buildings_.end()) {
    building = iter->second;
    this->RemoveBuilding(building);
  }

  return building;
}

Building *Scene::GetBuilding(int32_t id) {
  Building *building = NULL;

  BuildingHashmap::iterator iter = this->buildings_.find(id);
  if (iter != this->buildings_.end()) {
    building = iter->second;
  }

  return building;
}

Scene::BuildingSet *Scene::GetBuildings(const ::protocol::Position &pos) {
  Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return NULL;
  }

  return grid->GetBuildings();
}

const Scene::BuildingSet *Scene::GetBuildings(const ::protocol::Position &pos) const {
  const Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return NULL;
  }

  return grid->GetBuildings();
}

bool Scene::AddWarrior(Warrior *warrior) {
  WarriorHashmap::iterator iter = this->warriors_.find(warrior->GetId());
  if (iter != this->warriors_.end()) {
    return false;
  }

  ::protocol::Position pos = warrior->GetPos();
  Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return false;
  }

  if (grid->AddWarrior(warrior) == false) {
    return false;
  }

  this->warriors_.insert(std::make_pair(warrior->GetId(), warrior));

  return true;
}

void Scene::RemoveWarrior(Warrior *warrior) {
  ::protocol::Position pos = warrior->GetPos();
  Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return;
  }

  grid->RemoveWarrior(warrior);
  this->warriors_.erase(warrior->GetId());
}

Warrior *Scene::RemoveWarrior(int32_t id) {
  Warrior *warrior = NULL;

  WarriorHashmap::iterator iter = this->warriors_.find(id);
  if (iter != this->warriors_.end()) {
    warrior = iter->second;
    this->RemoveWarrior(warrior);
  }

  return warrior;
}

Warrior *Scene::GetWarrior(int32_t id) {
  Warrior *warrior = NULL;

  WarriorHashmap::iterator iter = this->warriors_.find(id);
  if (iter != this->warriors_.end()) {
    warrior = iter->second;
  }

  return warrior;
}

Scene::WarriorSet *Scene::GetWarriors(const ::protocol::Position &pos) {
  Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return NULL;
  }

  return grid->GetWarriors();
}

const Scene::WarriorSet *Scene::GetWarriors(const ::protocol::Position &pos) const {
  const Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return NULL;
  }

  return grid->GetWarriors();
}

void Scene::SearchPath(const ::protocol::Position &src_pos,
    const ::protocol::Position &dest_pos, PositionVector &paths) {
  // TODO:
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
