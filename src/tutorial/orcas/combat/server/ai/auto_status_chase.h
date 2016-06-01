#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_CHASE_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_CHASE_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/ai/auto_status.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class AutoStatusChase : public AutoStatus {
 public:
  AutoStatusChase(Auto *host);
  virtual ~AutoStatusChase();

  virtual void Start();
  virtual void Stop();
  virtual type GetType() const;

 private:
  void SetRefindPathTimer();

  void OnTimerRefindPath(int64_t timer_id);
  void OnEventSceneMoveStep(const ProtoMessage *data);
  void OnEventCombatDeath(const ProtoMessage *data);
  void OnEventCombatConvertCamp(const ProtoMessage *data);

  static const int kRefindPathMsec_ = 600;

  bool refind_path_;
  int64_t timer_id_refind_path_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(AutoStatusChase);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_CHASE_H
