#include "tutorial/orcas/combat/server/ai/building_manager.h"

#include "tutorial/orcas/combat/server/configs.h"
#include "tutorial/orcas/combat/server/ai/building.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

MYSYA_SINGLETON_IMPL(BuildingManager);

BuildingManager::BuildingManager()
  : pool_(Configs::GetInstance()->combat_building_initial_size_,
      Configs::GetInstance()->combat_building_extend_size_) {}
BuildingManager::~BuildingManager() {}

Building *BuildingManager::Allocate() {
  return this->pool_.Allocate();
}

void BuildingManager::Deallocate(Building *building) {
  this->pool_.Deallocate(building);
}

bool BuildingManager::Add(Building *building) {
  BuildingHashmap::const_iterator iter = this->buildings_.find(
      building->GetGlobalId());
  if (iter != this->buildings_.end()) {
    return false;
  }

  this->buildings_.insert(std::make_pair(building->GetGlobalId(), building));
  return true;
}

Building *BuildingManager::Get(const GlobalId &global_id) {
  Building *building = NULL;

  BuildingHashmap::iterator iter = this->buildings_.find(global_id);
  if (iter != this->buildings_.end()) {
    building = iter->second;
  }

  return building;
}

Building *BuildingManager::Get(int32_t combat_id, int32_t id) {
  return this->Get(GlobalId(combat_id, id));
}

Building *BuildingManager::Remove(const GlobalId &global_id) {
  Building *building = NULL;

  BuildingHashmap::iterator iter = this->buildings_.find(global_id);
  if (iter != this->buildings_.end()) {
    building = iter->second;
    this->buildings_.erase(iter);
  }

  return building;
}

Building *BuildingManager::Remove(int32_t combat_id, int32_t id) {
  return this->Remove(GlobalId(combat_id, id));
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
