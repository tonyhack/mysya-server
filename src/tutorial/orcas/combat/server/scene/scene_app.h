#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_APP_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_APP_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/scene/combat_event_handler.h"
#include "tutorial/orcas/combat/server/scene/entity_builder.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppServer;
class EventDispatcher;

namespace scene {

class SceneApp {
 public:
  bool Initialize(AppServer *host);
  void Finalize();

  AppServer *GetHost();
  EventDispatcher *GetEventDispatcher();
  CombatEventHandler *GetCombatEventHandler();
  EntityBuilder *GetEntityBuilder();

 private:
  AppServer *host_;

  CombatEventHandler combat_event_handler_;
  EntityBuilder entity_builder_;

  MYSYA_SINGLETON(SceneApp);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_APP_H
