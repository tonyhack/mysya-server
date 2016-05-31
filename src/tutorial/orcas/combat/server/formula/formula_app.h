#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_FORMULA_FORMULA_APP_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_FORMULA_FORMULA_APP_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/formula/require_handler.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppServer;
class EventDispatcher;
class RequireDispatcher;
class VoteDispatcher;

namespace formula {

class FormulaApp {
 public:
  bool Initialize(AppServer *host);
  void Finalize();

  AppServer *GetHost();
  EventDispatcher *GetEventDispatcher();
  RequireDispatcher *GetRequireDispatcher();
  VoteDispatcher *GetVoteDispatcher();

 private:
  AppServer *host_;
  RequireHandler require_handler_;

  MYSYA_SINGLETON(FormulaApp);
};

}  // namespace formula
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_FORMULA_FORMULA_APP_H
