#include "tutorial/orcas/combat/server/scene/building.h"

#include "tutorial/orcas/combat/server/combat_building_field.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

Building::Building()
  : scene_(NULL), host_(NULL) {}
Building::~Building() {}

bool Building::Initialize(CombatBuildingField *host, Scene *scene) {
  this->host_ = host;
  this->scene_ = scene;

  return true;
}

void Building::Finalize() {
  this->host_ = NULL;
  this->scene_ = NULL;
}

int32_t Building::GetId() const {
  return this->host_->GetFields().id();
}

::protocol::Position Building::GetPos() {
  ::protocol::Position pos;
  pos.set_x(this->host_->GetFields().pos_x());
  pos.set_y(this->host_->GetFields().pos_y());
  return pos;
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
