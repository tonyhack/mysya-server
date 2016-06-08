#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AI_APP_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AI_APP_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/ai/combat_event_handler.h"
#include "tutorial/orcas/combat/server/ai/require_handler.h"
#include "tutorial/orcas/combat/server/ai/scene_event_handler.h"
#include "tutorial/orcas/combat/server/ai/vote_handler.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppServer;
class EventDispatcher;
class RequireDispatcher;
class VoteDispatcher;

namespace ai {

class AiApp {
 public:
  bool Initialize(AppServer *host);
  void Finalize();

  AppServer *GetHost();
  EventDispatcher *GetEventDispatcher();
  RequireDispatcher *GetRequireDispatcher();
  VoteDispatcher *GetVoteDispatcher();

 private:
  AppServer *host_;
  CombatEventHandler combat_event_handler_;
  RequireHandler require_handler_;
  SceneEventHandler scene_event_handler_;
  VoteHandler vote_handler_;

  MYSYA_SINGLETON(AiApp);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AI_APP_H
