#include "tutorial/orcas/combat/server/apps.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

Apps::Apps(AppServer *host) : host_(host) {}
Apps::~Apps() {}

bool Apps::Initialize() {
  if (scene::SceneApp::GetInstance()->Initialize(
        this->host_) == false) {
    MYSYA_ERROR("SceneApp::Initialize failed.");
    return false;
  }

  return true;
}

void Apps::Finalize() {
  scene::SceneApp::GetInstance()->Finalize();
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
