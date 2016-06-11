#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_STATUS_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_STATUS_H

#include <stdint.h>

#include <map>
#include <functional>

#include <mysya/util/class_util.h>

#include "tutorial/orcas/protocol/cc/building.pb.h"

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class Building;

class BuildingStatus {
 public:
  typedef ::google::protobuf::Message ProtoMessage;

  BuildingStatus(Building *host);
  virtual ~BuildingStatus();

  virtual void Start() = 0;
  virtual void Stop() = 0;
  virtual ::protocol::BuildingStatusType GetType() const = 0;
  virtual void OnRecoverHp() = 0;

  virtual void OnEvent(int type, const ProtoMessage *event) = 0;

 protected:
  bool GotoStatus(int status);

  Building *host_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(BuildingStatus);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_BUILDING_STATUS_H
