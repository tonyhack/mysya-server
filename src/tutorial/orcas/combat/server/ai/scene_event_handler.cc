#include "tutorial/orcas/combat/server/ai/scene_event_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/ai/ai_app.h"
#include "tutorial/orcas/combat/server/ai/event_observer.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_scene.pb.h"

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

  return true;
}

void SceneEventHandler::Finalize() {
  EVENT_DISPATCHER()->Detach(this->event_token_scene_move_step_);
}

#undef EVENT_DISPATCHER

void SceneEventHandler::OnEventSceneMoveStep(const ProtoMessage *data) {
  event::EventSceneMoveStep *event = (event::EventSceneMoveStep *)data;

  EventObserver::GetInstance()->Dispatch(event->combat_id(), event->warrior_id(),
      event::EVENT_SCENE_MOVE_STEP, event);
}

#undef AI_APP

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
