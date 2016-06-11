#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_MANAGER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_MANAGER_H

#include <unordered_map>

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/pool_template.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class Building;

class BuildingManager {
 public:
  typedef std::pair<int32_t, int32_t> GlobalId;

  struct GlobalIdHash {
    std::size_t operator()(const GlobalId &key) const {
      return ((size_t)key.first << 32) + (size_t)key.second;
    }
  };

  typedef std::unordered_map<GlobalId, Building *, GlobalIdHash> BuildingHashmap;

  Building *Allocate();
  void Deallocate(Building *building);

  bool Add(Building *building);
  Building *Get(const GlobalId &global_id);
  Building *Get(int32_t combat_id, int32_t id);
  Building *Remove(const GlobalId &global_id);
  Building *Remove(int32_t combat_id, int32_t id);

 private:
  BuildingHashmap buildings_;
  PoolTemplate<Building> pool_;

  MYSYA_SINGLETON(BuildingManager);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_MANAGER_H
