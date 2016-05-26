#include "tutorial/orcas/combat/server/ai/auto_status_attack.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusAttack::AutoStatusAttack()
  : AutoStatus(host) {}
AutoStatusAttack::~AutoStatusAttack() {}

void AutoStatusAttack::Start() {}
void AutoStatusAttack::Stop() {}

void AutoStatusAttack::OnEvent(int type, ProtoMessage *data) {}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
