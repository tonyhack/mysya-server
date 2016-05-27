#include "tutorial/orcas/combat/server/ai/auto_status_chase.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusChase::AutoStatusChase(Auto *host)
  : AutoStatus(host) {}
AutoStatusChase::~AutoStatusChase() {}

void AutoStatusChase::Start() {}
void AutoStatusChase::Stop() {}

void AutoStatusChase::OnEvent(int type, ProtoMessage *data) {}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
