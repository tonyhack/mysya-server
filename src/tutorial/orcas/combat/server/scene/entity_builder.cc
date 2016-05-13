#include "tutorial/orcas/combat/server/scene/entity_builder.h"

#include "tutorial/orcas/combat/server/scene/building.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"
#include "tutorial/orcas/combat/server/scene/warrior.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

EntityBuilder::EntityBuilder(SceneApp *host)
  : host_(host) {}
EntityBuilder::~EntityBuilder() {}

Building *EntityBuilder::AllocateBuilding() {
  return new (std::nothrow) Building();
}

void EntityBuilder::DeallocateBuilding(Building *building) {
  delete building;
}

Warrior *EntityBuilder::AllocateWarrior() {
  return new (std::nothrow) Warrior();
}

void EntityBuilder::DeallocateWarrior(Warrior *warrior) {
  delete warrior;
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
