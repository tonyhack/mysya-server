#include "tutorial/orcas/combat/server/scene/scene_app.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/configs.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/scene/scene_manager.h"

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

  if (SceneManager::GetInstance()->LoadConfig(
        Configs::GetInstance()->conf_path_ + "map.xml") == false) {
    MYSYA_ERROR("[SCENE] SceneManager::LoadConfig(%s) failed.",
        (Configs::GetInstance()->conf_path_ + "map.xml").data());
    return false;
  }

  this->combat_event_handler_.Initialize();
  this->combat_require_handler_.Initialize();

  return true;
}

void SceneApp::Finalize() {
  this->combat_require_handler_.Finalize();
  this->combat_event_handler_.Finalize();

  this->host_ = NULL;
}

AppServer *SceneApp::GetHost() {
  return this->host_;
}

EventDispatcher *SceneApp::GetEventDispatcher() {
  return this->host_->GetEventDispatcher();
}

EntityBuilder *SceneApp::GetEntityBuilder() {
  return &this->entity_builder_;
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
