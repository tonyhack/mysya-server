#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_STATUS_HOST_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_STATUS_HOST_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/ai/building_status.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class BuildingStatusHost : public BuildingStatus {
 public:
  BuildingStatusHost(Building *host);
  virtual ~BuildingStatusHost();

  virtual void Start();
  virtual void Stop();

  virtual ::protocol::BuildingStatusType GetType() const;
  virtual void OnRecoverHp();

  virtual void OnEvent(int type, const ProtoMessage *event);

 private:
  void OnEventCombatConvertCamp(const ProtoMessage *data);

  MYSYA_DISALLOW_COPY_AND_ASSIGN(BuildingStatusHost);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_STATUS_HOST_H
