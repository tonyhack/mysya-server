#include "tutorial/orcas/combat/message_dispatcher.h"

#include <google/protobuf/message.h>

#include "tutorial/orcas/combat/transport_channel.h"

namespace tutorial {
namespace orcas {
namespace combat {

MessageDispatcher::MessageDispatcher() {}
MessageDispatcher::~MessageDispatcher() {}

void MessageDispatcher::SetMessageCallback(const std::string &type_name,
    const MessageCallback &cb) {
  this->message_cbs_[type_name] = cb;
}

void MessageDispatcher::ResetMessageCallback(const std::string &type_name) {
  this->message_cbs_.erase(type_name);
}

void MessageDispatcher::Dispatch(TransportChannel *transport_channel,
    const ::google::protobuf::Message *message) {
  MessageCallbackHashmap::iterator iter = this->message_cbs_.find(
      message->GetTypeName());
  if (iter == this->message_cbs_.end()) {
    return;
  }

  iter->second(transport_channel, message);
}

}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
