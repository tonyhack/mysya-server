#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_MANAGER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_MANAGER_H

#include <unordered_map>

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/pool_template.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class Auto;

class AutoManager {
 public:
  typedef std::pair<int32_t, int32_t> AutoGlobalId;
  struct AutoGlobalIdHash {
    std::size_t operator()(const AutoGlobalId &key) const {
      return ((size_t)key.first << 32) + (size_t)key.second;
    }
  };
  typedef std::unordered_map<AutoGlobalId, Auto *, AutoGlobalIdHash> AutoHashmap;

  Auto *Allocate();
  void Deallocate(Auto *autoz);

  bool Add(Auto *autoz);
  Auto *Get(const AutoGlobalId &global_id);
  Auto *Get(int32_t combat_id, int32_t id);
  Auto *Remove(const AutoGlobalId &global_id);
  Auto *Remove(int32_t combat_id, int32_t id);

 private:
  AutoHashmap autoes_;
  PoolTemplate<Auto> pool_;

  MYSYA_SINGLETON(AutoManager);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_MANAGER_H
