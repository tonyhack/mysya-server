#include "tutorial/orcas/gateway/server/message_dispatcher.h"

#include "tutorial/orcas/gateway/server/actor.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

MessageDispatcher::MessageDispatcher() {}
MessageDispatcher::~MessageDispatcher() {}

void MessageDispatcher::SetMessageCallback(int type, const MessageCallback &cb) {
  this->message_cbs_[type] = cb;
}

void MessageDispatcher::ResetMessageCallback(int type) {
  this->message_cbs_.erase(type);
}

void MessageDispatcher::Dispatch(Actor *actor, int type, const char *data, int size) {
  MessageCallbackHashmap::iterator iter = this->message_cbs_.find(type);
  if (iter == this->message_cbs_.end()) {
    return;
  }

  iter->second(actor, data, size);
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
