#include "tutorial/orcas/combat/server/scene/scene_app.h"

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

MYSYA_SINGLETON_IMPL(SceneApp);

SceneApp::SceneApp()
  : host_(NULL), entity_builder_(this) {}
SceneApp::~SceneApp() {}

bool SceneApp::Initialize(AppServer *host) {
  this->host_ = host;

  return true;
}

void SceneApp::Finalize() {
  this->host_ = NULL;
}

AppServer *SceneApp::GetHost() {
  return this->host_;
}

EventDispatcher *SceneApp::GetEventDispatcher() {
  return this->host_->GetEventDispatcher();
}

CombatEventHandler *SceneApp::GetCombatEventHandler() {
  return &this->combat_event_handler_;
}

EntityBuilder *SceneApp::GetEntityBuilder() {
  return &this->entity_builder_;
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
