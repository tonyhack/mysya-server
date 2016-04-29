#include "tutorial/orcas/combat/server/configs.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

MYSYA_SINGLETON_IMPL(Configs);

Configs::Configs()
  : server_id_(0), listen_host_(0) {}
Configs::~Configs() {}

bool Configs::Load(const std::string &file) {
  return true;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
