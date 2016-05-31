#include "tutorial/orcas/combat/server/apps.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/formula/formula_app.h"
#include "tutorial/orcas/combat/server/scene/scene_app.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

Apps::Apps(AppServer *host) : host_(host) {}
Apps::~Apps() {}

bool Apps::Initialize() {
  if (ai::AiApp::GetInstance()->Initialize(
        this->host_) == false) {
    MYSYA_ERROR("AiApp::Initialize() failed.");
    return false;
  }

  if (formula::FormulaApp::GetInstance()->Initialize(
        this->host_) == false) {
    MYSYA_ERROR("FormulaApp::Initialize() failed.");
    return false;
  }

  if (scene::SceneApp::GetInstance()->Initialize(
        this->host_) == false) {
    MYSYA_ERROR("SceneApp::Initialize() failed.");
    return false;
  }

  return true;
}

void Apps::Finalize() {
  scene::SceneApp::GetInstance()->Finalize();
  formula::FormulaApp::GetInstance()->Finalize();
  ai::AiApp::GetInstance()->Finalize();
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
