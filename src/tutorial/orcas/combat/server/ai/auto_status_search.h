#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_SEARCH_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_SEARCH_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/ai/auto_status.h"

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
  virtual type GetType() const;

 private:
  void OnTimerSearch(int64_t timer_id);

  static const int kSearchExpireMsec_ = 200;

  int64_t timer_id_search_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(AutoStatusSearch);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_SEARCH_H
