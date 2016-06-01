#include "tutorial/orcas/combat/server/scene/scene.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_warrior_field.h"
#include "tutorial/orcas/combat/server/scene/building.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"
#include "tutorial/orcas/combat/server/scene/scene_manager.h"
#include "tutorial/orcas/combat/server/scene/warrior.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

bool Scene::Node::operator <(const Node &other) const {
  return this->f_ < other.f_;
}

int Scene::Node::NeighbroGCost(const Node &other) const {
  if (this->pos_.x() == other.pos_.x() ||
      this->pos_.y() == other.pos_.y()) {
    return 10;
  } else {
    return 14;
  }
}

int Scene::Node::HeursiticConstEstimate(const Node &other) const {
  int dx = pos_.x() - other.pos_.x();
  int dy = pos_.y() - other.pos_.y();

  return (int)(sqrt(dx *dx + dy * dy) * 10);
}

Scene::Scene()
  : map_id_(0), host_(NULL),
    height_(0), width_(0),
    grid_size_(0), grids_(NULL),
    timer_id_print_status_image_(-1) {}

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

  for (int y = 0; y < this->height_; ++y) {
    for (int x = 0; x < this->width_; ++x) {
      Node node;
      node.pos_.set_x(x);
      node.pos_.set_y(y);
      node.walkable_ = (conf->blocks_[y * this->width_ + x] == '0');
      this->nodes_.push_back(node);
    }
  }

  return true;
}

void Scene::Deallocate() {
  delete [] this->grids_;
}

bool Scene::Initialize(CombatField *host) {
  this->host_ = host;

  this->timer_id_print_status_image_ = SceneApp::GetInstance()->GetHost()->StartTimer(
      1000, std::bind(&Scene::OnTimerPrintStatusImage, this, std::placeholders::_1), -1);

  MYSYA_DEBUG("[SCENE] Scene(%d) Initialize success.", this->GetId());

  return true;
}

void Scene::Finalize() {
  if (this->timer_id_print_status_image_ != -1) {
    SceneApp::GetInstance()->GetHost()->StopTimer(this->timer_id_print_status_image_);
  }

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

Grid *Scene::GetGrid(const Position &pos) {
  if (pos.x() < 0 || pos.x() >= this->width_ ||
      pos.y() < 0 || pos.y() >= this->height_) {
    return NULL;
  }

  return &this->grids_[pos.y()*this->width_+pos.x()];
}

const Grid *Scene::GetGrid(const Position &pos) const {
  if (pos.x() < 0 || pos.x() >= this->width_ ||
      pos.y() < 0 || pos.y() >= this->height_) {
    return NULL;
  }

  return &this->grids_[pos.y()*this->width_+pos.x()];
}

Grid *Scene::GetGrid(int32_t x, int32_t y) {
  if (x < 0 || x >= this->width_ ||
      y < 0 || y >= this->height_) {
    return NULL;
  }

  return &this->grids_[y*this->width_+x];
}

const Grid *Scene::GetGrid(int32_t x, int32_t y) const {
  if (x < 0 || x >= this->width_ ||
      y < 0 || y >= this->height_) {
    return NULL;
  }

  return &this->grids_[y*this->width_+x];
}

bool Scene::GetWalkable(const Position &pos) const {
  const Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return false;
  }

  return grid->GetWalkable();
}

bool Scene::GetWalkable(int32_t x, int32_t y) const {
  Position pos;
  pos.set_x(x);
  pos.set_y(y);

  return this->GetWalkable(pos);
}

bool Scene::GetNearlyWalkablePos(const Position &pos, Position &nearly_pos) const {
  int start_x = std::max(pos.x() - 1, 0);
  int end_x = std::min(pos.x() + 1, this->width_ - 1);
  int start_y = std::max(pos.y() - 1, 0);
  int end_y = std::min(pos.y() + 1, this->height_ - 1);

  for (int y = start_y; y <= end_y; ++y) {
    for (int x = start_x; x < end_x; ++x) {
      nearly_pos.set_x(x);
      nearly_pos.set_y(y);
      if (this->GetWalkable(nearly_pos) == true) {
        return true;
      }
    }
  }

  return false;
}

bool Scene::GetNearlyWalkablePos(int32_t x, int32_t y, Position &nearly_pos) const {
  Position pos;
  pos.set_x(x);
  pos.set_y(y);

  return this->GetNearlyWalkablePos(x, y, nearly_pos);
}

bool Scene::AddBuilding(Building *building) {
  BuildingHashmap::iterator iter = this->buildings_.find(building->GetId());
  if (iter != this->buildings_.end()) {
    return false;
  }

  Position pos = building->GetPos();
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
  Position pos = building->GetPos();
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

Scene::BuildingSet *Scene::GetBuildings(const Position &pos) {
  Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return NULL;
  }

  return grid->GetBuildings();
}

const Scene::BuildingSet *Scene::GetBuildings(const Position &pos) const {
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

  Position pos = warrior->GetPos();
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
  Position pos = warrior->GetPos();
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

Scene::WarriorSet *Scene::GetWarriors(const Position &pos) {
  Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return NULL;
  }

  return grid->GetWarriors();
}

const Scene::WarriorSet *Scene::GetWarriors(const Position &pos) const {
  const Grid *grid = this->GetGrid(pos);
  if (grid == NULL) {
    return NULL;
  }

  return grid->GetWarriors();
}

bool Scene::GetNeighbors(const Position &pos, int32_t range,
    BuildingVector &buildings, WarriorVector &warriors) {
  buildings.clear();
  warriors.clear();

  for (int i = 0; i <= range; ++i) {
    int start_x = std::max(pos.x() - i, 0);
    int end_x = std::min(pos.x() + i, this->width_ - i);
    int start_y = std::max(pos.y() - i, 0);
    int end_y = std::min(pos.y() + i, this->height_ - i);

    for (int y = start_y; y <= end_y; ++y) {
      for (int x = start_x; x <= end_x; ++x) {
        Grid *grid = this->GetGrid(x, y);
        if (grid == NULL) {
          continue;
        }

        const WarriorSet *grid_warriors = grid->GetWarriors();
        if (grid_warriors != NULL) {
          for (WarriorSet::const_iterator iter = grid_warriors->begin();
              iter != grid_warriors->end(); ++iter) {
            warriors.push_back(*iter);
          }
        }

        const BuildingSet *grid_buildings = grid->GetBuildings();
        if (grid_buildings != NULL) {
          for (BuildingSet::const_iterator iter = grid_buildings->begin();
              iter != grid_buildings->end(); ++iter) {
            buildings.push_back(*iter);
          }
        }
      }
    }
  }

  return true;
}

void Scene::SearchPath(const Position &begin_pos, const Position &end_pos,
    PositionVector &paths) {
  if (begin_pos.x() < 0 || begin_pos.x() >= this->width_ ||
      begin_pos.y() < 0 || begin_pos.y() >= this->height_) {
    MYSYA_ERROR("[SCENE] begin_pos(%d,%d) is invalid.",
        begin_pos.x(), begin_pos.y());
    return;
  }
  if (end_pos.x() < 0 || end_pos.x() >= this->width_ ||
      end_pos.y() < 0 || end_pos.y() >= this->height_) {
    MYSYA_ERROR("[SCENE] end_pos(%d,%d) is invalid.",
        end_pos.x(), end_pos.y());
    return;
  }

  for (size_t i = 0; i < this->nodes_.size(); ++i) {
    Node *node = &this->nodes_[i];
    node->f_ = 0;
    node->g_ = 0;
    node->open_list_pos_ = 0;
    node->close_list_pos_ = 0;
    node->parent_ = NULL;
  }

  this->start_node_ = this->GetNode(begin_pos);
  this->end_node_ = this->GetNode(end_pos);
  this->open_list_.clear();
  this->open_list_.push_back(NULL);

  this->start_node_->g_ = 0;
  this->start_node_->f_ = this->start_node_->g_ +
      this->start_node_->HeursiticConstEstimate(*this->end_node_);
  this->InsertOpenList(this->start_node_);

  NodePtrVector neighbor_nodes;
  neighbor_nodes.reserve(8);

  while (this->IsOpenListEmpty() == false) {
    Node *cur_node = this->GetMinFScoreNodeInOpenList();
    if (cur_node == this->end_node_) {
      this->ConstructResultPath(paths);
      return;
    }

    this->DeleteMinFScoreNodeInOpenList();
    this->InsertCloseList(cur_node);

    this->GetNeighborNodes(cur_node, neighbor_nodes);

    for (size_t i = 0; i < neighbor_nodes.size(); ++i) {
      Node *neighbor = neighbor_nodes[i];

      bool is_in_open_list = this->IsInOpenList(neighbor);
      bool is_in_close_list = this->IsInCloseList(neighbor);
      int g_cal = cur_node->g_ + cur_node->NeighbroGCost(*neighbor);

      if (is_in_close_list && g_cal >= neighbor->g_) {
        continue;
      }

      if (is_in_open_list == false || g_cal < neighbor->g_) {
        neighbor->parent_ = cur_node;
        neighbor->g_ = g_cal;
        neighbor->f_ = neighbor->g_ +
          neighbor->HeursiticConstEstimate(*this->end_node_);

        if (is_in_open_list == false) {
          this->InsertOpenList(neighbor);
        } else {
          this->ResortOpenList(neighbor);
        }
      }
    }
  }
}

void Scene::PrintSearchPath(const Position &begin_pos, const Position &end_pos) {
  const SceneConf *conf =
    SceneManager::GetInstance()->GetSceneConf(this->map_id_);
  if (conf == NULL) {
    MYSYA_ERROR("[SCENE] SceneManager::GetSceneConf(%d) failed.", this->map_id_);
    return;
  }

  PositionVector result;
  this->SearchPath(begin_pos, end_pos, result);

  std::string map_string = conf->blocks_;
  if (result.empty() == true) {
    MYSYA_ERROR("[SCENE] SearchPath() failed.");
    return;
  }

  for (size_t i = 0; i < result.size(); ++i) {
    if (i == 0) {
      map_string[result[i].y() * this->width_ + result[i].x()] = 'B';
    } else if (i == result.size() - 1) {
      map_string[result[i].y() * this->width_ + result[i].x()] = 'E';
    } else {
      map_string[result[i].y() * this->width_ + result[i].x()] = 'X';
    }
  }

  for (int y = 0; y < this->height_; ++y) {
    for (int x = 0; x < this->width_; ++x) {
      char c = map_string[y * this->width_ + x];

      if (c == 'X' || c == 'B' || c == 'E') {
        ::printf("\033[;32m");
        ::printf("%c", c);
        ::printf("\033[0m");
      } else {
        ::printf("%c", c);
      }
    }
    printf("\n");
  }
}

void Scene::PrintStatusImage() {
  return;
  ::printf("========================================================= PrintStatusImage =========================================================\n");
  const char *kWarriorSymbols = "abcdefghijklmnopqrstuvwxyz";
  const char kBuildingSymbol = 'B';
  const std::string kCampColors[] = {
    "\033[1;33m", "\033[1;36m", "\033[1;35m", "\033[1;34m", };
  const char kBlockSymbol = 'X';
  size_t warrior_symbols_pos = 0;
  size_t camp_colors_pos = 0;

  std::map<int, char> warrior_symbols;
  std::map<int, std::string> camp_colors;

  for (int y = 0; y < this->height_; ++y) {
    for (int x = 0; x < this->width_; ++x) {
      typedef Grid::BuildingSet BuildingSet;
      typedef Grid::WarriorSet WarriorSet;

      Grid *grid = &this->grids_[y * this->width_ + x];

      const BuildingSet *buildings = grid->GetBuildings();
      const WarriorSet *warriors = grid->GetWarriors();

      if (buildings->empty() == false) {
        Building *building = *buildings->begin();
        ::protocol::CombatBuildingFields &fields = building->GetHost()->GetFields();
        std::map<int, std::string>::iterator camp_iter = camp_colors.find(fields.camp_id());
        if (camp_iter == camp_colors.end()) {
          camp_iter = camp_colors.insert(std::make_pair(fields.camp_id(), kCampColors[camp_colors_pos++])).first;
        }
        ::printf("%s", camp_iter->second.data());
        ::printf("%c", kBuildingSymbol);
        ::printf("\033[0m");
      } else if (warriors->empty() == false) {
        Warrior *warrior = *warriors->begin();
        ::protocol::CombatWarriorFields &fields = warrior->GetHost()->GetFields();
        std::map<int, char>::iterator warrior_iter = warrior_symbols.find(fields.conf_id());
        if (warrior_iter == warrior_symbols.end()) {
          warrior_iter = warrior_symbols.insert(std::make_pair(fields.conf_id(), kWarriorSymbols[warrior_symbols_pos++])).first;
        }
        std::map<int, std::string>::iterator camp_iter = camp_colors.find(fields.camp_id());
        if (camp_iter == camp_colors.end()) {
          camp_iter = camp_colors.insert(std::make_pair(fields.camp_id(), kCampColors[camp_colors_pos++])).first;
        }
        ::printf("%s", camp_iter->second.data());
        ::printf("%c", warrior_iter->second);
        ::printf("\033[0m");
      } else if (grid->GetWalkable() == false) {
        ::printf("%c", kBlockSymbol);
      } else {
        ::printf(" ");
      }
    }
    printf("\n");
  }
  ::printf("========================================================= PrintStatusImage =========================================================\n");
}

void Scene::OnTimerPrintStatusImage(int64_t timer_id) {
  /*
  Position pos1, pos2;
  pos1.set_x(46);
  pos1.set_y(12);
  pos2.set_x(79);
  pos2.set_y(13);
  this->PrintSearchPath(pos1, pos2);
  */
  // this->PrintStatusImage();
}

Scene::Node *Scene::GetNode(const Position &pos) {
  return this->GetNode(pos.x(), pos.y());
}

Scene::Node *Scene::GetNode(int32_t x, int32_t y) {
  return &this->nodes_[y * this->width_ + x];
}

void Scene::GetNeighborNodes(const Node *node, NodePtrVector &neighbor_nodes) {
  neighbor_nodes.clear();

  int start_x = std::max(node->pos_.x() - 1, 0);
  int end_x = std::min(node->pos_.x() + 1, this->width_ - 1);
  int start_y = std::max(node->pos_.y() - 1, 0);
  int end_y = std::min(node->pos_.y() + 1, this->height_ - 1);

  for (int y = start_y; y <= end_y; ++y) {
    for (int x = start_x; x <= end_x; ++x) {
      if (x != node->pos_.x() || y != node->pos_.y()) {
        Node *neighbor = this->GetNode(x, y);
        if (neighbor->walkable_) {
          neighbor_nodes.push_back(neighbor);
        }
      }
    }
  }
}

bool Scene::IsInOpenList(const Node *node) const {
  return node->open_list_pos_;
}

bool Scene::IsOpenListEmpty() const {
  return this->open_list_.size() <= 1;
}

void Scene::InsertOpenList(Node *node) {
  size_t cur_index = this->open_list_.size();
  this->open_list_.push_back(node);
  node->open_list_pos_ = cur_index;

  for (;;) {
    size_t parent_index = cur_index / 2;

    if (parent_index == 0) {
      break;
    }

    if (*this->open_list_[parent_index] < *this->open_list_[cur_index]) {
      break;
    }

    this->open_list_[parent_index]->open_list_pos_ = cur_index;
    this->open_list_[cur_index]->open_list_pos_ = parent_index;
    std::swap(this->open_list_[parent_index], this->open_list_[cur_index]);

    cur_index = parent_index;
  }
}

Scene::Node *Scene::GetMinFScoreNodeInOpenList() {
  if (this->IsOpenListEmpty() == true) {
    return NULL;
  }

  return this->open_list_[1];
}

void Scene::DeleteMinFScoreNodeInOpenList() {
  if (this->IsOpenListEmpty()) {
    return;
  }

  this->open_list_[1]->open_list_pos_ = 0;
  this->open_list_[1] = this->open_list_.back();
  this->open_list_.pop_back();

  size_t cur_index = 1;

  for (;;) {
    size_t child_index = cur_index * 2;

    if (child_index >= this->open_list_.size()) {
      break;
    }

    if (child_index + 1 < this->open_list_.size() &&
        *this->open_list_[child_index + 1] < *this->open_list_[child_index]) {
      ++child_index;
    }

    if (*this->open_list_[cur_index] < *this->open_list_[child_index]) {
      break;
    }

    this->open_list_[cur_index]->open_list_pos_ = child_index;
    this->open_list_[child_index]->open_list_pos_ = cur_index;
    std::swap(this->open_list_[cur_index], this->open_list_[child_index]);

    cur_index = child_index;
  }
}

void Scene::ResortOpenList(Node *node) {
  size_t cur_index = node->open_list_pos_;

  for (;;) {
    size_t parent_index = cur_index / 2;

    if (0 == parent_index) {
      break;
    }
    if (*this->open_list_[parent_index] < *this->open_list_[cur_index]) {
      break;
    }

    this->open_list_[parent_index]->open_list_pos_ = cur_index;
    this->open_list_[cur_index]->open_list_pos_ = parent_index;
    std::swap(this->open_list_[parent_index], this->open_list_[cur_index]);

    cur_index = parent_index;
  }
}

bool Scene::IsInCloseList(const Node *node) const {
  return node->close_list_pos_;
}

void Scene::InsertCloseList(Node *node) {
  node->close_list_pos_ = 1;
}

void Scene::ConstructResultPath(PositionVector &result) {
  for (Node *node = end_node_;
      node != this->start_node_ && node != NULL;
      node = node->parent_) {
    result.push_back(node->pos_);
  }
  std::reverse(result.begin(), result.end());
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
