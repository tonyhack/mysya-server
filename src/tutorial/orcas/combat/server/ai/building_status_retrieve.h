#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_STATUS_RETRIEVE_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_STATUS_RETRIEVE_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/server/ai/building_status.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class BuildingStatusRetrieve : public BuildingStatus {
 public:
  BuildingStatusRetrieve(Building *host);
  virtual ~BuildingStatusRetrieve();

  virtual void Start();
  virtual void Stop();

  virtual ::protocol::BuildingStatusType GetType() const;
  virtual void OnRecoverHp();

  virtual void OnEvent(int type, const ProtoMessage *event);

 private:
  MYSYA_DISALLOW_COPY_AND_ASSIGN(BuildingStatusRetrieve);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_STATUS_RETRIEVE_H
