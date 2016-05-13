#include "tutorial/orcas/combat/server/require_dispatcher.h"

#include <google/protobuf/message.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

RequireDispatcher::RequireDispatcher() {}
RequireDispatcher::~RequireDispatcher() {}

void RequireDispatcher::Attach(int32_t type, const RequireCallback &cb) {
  this->cbs_[type] = cb;
}

void RequireDispatcher::Detach(int32_t type) {
  this->cbs_.erase(type);
}

int RequireDispatcher::Dispatch(int32_t type, ProtoMessage *data) {
  RequireCallbackHashmap::iterator iter = this->cbs_.find(type);
  if (iter == this->cbs_.end()) {
    return -1;
  }

  return iter->second(data);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
