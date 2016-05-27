#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AI_APP_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AI_APP_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/ai/combat_vote_handler.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppServer;
class EventDispatcher;
class RequireDispatcher;

namespace ai {

class AiApp {
 public:
  bool Initialize(AppServer *hosts);
  void Finalize();

  AppServer *GetHost();
  EventDispatcher *GetEventDispatcher();
  RequireDispatcher *GetRequireDispatcher();

 private:
  AppServer *host_;
  CombatVoteHandler combat_vote_handler_;

  MYSYA_SINGLETON(AiApp);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AI_APP_H
