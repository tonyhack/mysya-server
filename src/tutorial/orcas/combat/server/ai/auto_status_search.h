#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_SEARCH_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATE_SEARCH_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/ai/auto_state.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class AutoStatusSearch : public AutoStatus {
 public:
  AutoStatusSearch(Auto *host);
  virtual ~AutoStatusSearch();

  virtual void Start();
  virtual void Stop();

  virtual void OnEvent(int type, ProtoMessage *data);

 private:
  void OnTimerSearch(int64_t timer_id);

  static const int kSearchExpireMsec_ = 100;

  int64_t timer_id_search_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(AutoStatusSearch);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_SEARCH_H
