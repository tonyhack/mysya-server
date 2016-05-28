#include "tutorial/orcas/combat/server/ai/scene_event_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

#define AI_APP() \
    AiApp::GetInstance()

SceneEventHandler::SceneEventHandler()
  : event_token_scene_move_step_(0) {}
SceneEventHandler::~SceneEventHandler() {}

#define EVENT_DISPATCHER \
  AI_APP()->GetHost()->GetEventDispatcher

bool SceneEventHandler::Initialize() {
  this->event_token_scene_move_step_ =
    EVENT_DISPATCHER()->Attach(event::EVENT_SCENE_MOVE_STEP, std::bind(
          &SceneEventHandler::OnEventSceneMoveStep, this, std::placeholders::_1));
}

void SceneEventHandler::Finalize() {
  EVENT_DISPATCHER()->Detach(this->event_token_scene_move_step_);
}

#undef EVENT_DISPATCHER

void SceneEventHandler::OnEventSceneMoveStep(const ProtoMessage *data) {
  event::EventSceneMoveStep *event = (event::EventSceneMoveStep *)data;

  Auto *autoz = AutoManager::GetInstance(event->combat_id(), event->warrior_id());
  if (autoz == NULL) {
    MYSYA_ERROR("[AI] AutoManager::Get(%d, %d) failed.",
        event->combat_id(), event->warrior_id());
    return;
  }

  AutoStatus *auto_status = autoz->GetPresentStatus();
  if (auto_status == NULL) {
    MYSYA_ERROR("[AI] Auto::GetPresentStatus() faild.");
    return;
  }

  auto_status->OnEvent(event::EVENT_SCENE_MOVE_STEP, data);
}

#undef AI_APP

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
