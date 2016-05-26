#include "tutorial/orcas/combat/server/ai/auto_status_move.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusMove::AutoStatusMove(Auto *host)
  : AutoStatus(host) {}
AutoStatusMove::~AutoStatusMove() {}

void AutoStatusMove::Start() {}
void AutoStatusMove::Stop() {}

void AutoStatusMove::OnEvent(int type, ProtoMessage *data) {}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
