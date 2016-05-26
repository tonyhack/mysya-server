#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_MANAGER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_MANAGER_H

#include <unordered_map>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class Auto;

class AutoManager {
 public:
  typedef std::unordered_map<int32_t, Auto *> AutoHashmap;

  Auto *Allocate(CombatWarriorField *host);
  void Deallocate(Auto *autoz);

  bool Add(Auto *autoz);
  Auto *Get(int32_t id);
  Auto *Remove(int32_t id);

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
